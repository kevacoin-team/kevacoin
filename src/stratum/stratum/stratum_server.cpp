// stratum_server.cpp
#include "stratum_server.h"
#include "blocktemplate.h"
#include "config.h"
#include "endpoint.h"
#include "job.h"
#include "miners_map.h"
#include "rpc.h"
#include "session.h"
#include "stratum/util/util.h"
#include "structures.h"
#include "univalue.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <iostream>
#include <regex>
#include <thread>
#include <util.h>
#include <shared_mutex>

namespace mp = boost::multiprecision;

namespace stratum
{

StratumServer::StratumServer(Config* config)
    : config(config), roundShares(0), blockTemplate(nullptr), minersMap(), sessions(), sessionsMutex(), minersMutex()
{
    try {
        auto client = std::make_shared<RPCClient>();

        rcli = client;

        LogPrintf("RPC Client initialized.\n");
    } catch (const std::exception& e) {
        LogPrintf("Failed to initialize RPCClient: %s\n", e.what());
    }

    // if (config->Address == "") {
    //     LogPrintf("Error stratum address not set.\n");
    // } else if (!util::ValidateAddress(config->Address)) {
    //     LogPrintf("Error with supplied stratum address, ensure it is a valid Kevacoin address.\n");
    // } else {
    fetchBlockTemplate();
    stopFlag = false;
    refreshThread = std::thread([this]() {
        while (!stopFlag) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            refreshBlockTemplate(true);
        }
    });
    // }
}

StratumServer::~StratumServer()
{
    for (auto& io_context : endpointIoServices) {
        io_context->stop();
    }

    for (auto& thread : endpointThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    stopFlag = true;
    if (refreshThread.joinable()) {
        refreshThread.join();
    }
}

std::pair<std::shared_ptr<JobReply>, std::shared_ptr<ErrorReply>> StratumServer::handleLoginRPC(std::shared_ptr<Session> session, const LoginParams& params)
{
    std::string address = session->extractWorkerId(params.login).first;
    std::string id = params.pass;

    // if (!util::ValidateAddress(address)) {
    //     LogPrintf("Invalid address %s used for login by %s.\n", address, session->ip);
    //     auto error = std::make_shared<ErrorReply>();
    //     error->code = -1;
    //     error->message = "Invalid address used for login";
    //     return {nullptr, error};
    // }

    auto t = currentBlockTemplate();
    if (!t) {
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Job not ready";
        return {nullptr, error};
    }

    std::shared_ptr<Miner> miner;
    {
        std::unique_lock<boost::shared_mutex> lock(minersMutex);
        auto result = minersMap.Get(id);
        auto retrievedMiner = result.first;
        auto found = result.second;
        if (found) {
            miner = retrievedMiner;
        }
    }

    if (!miner) {
        miner = std::make_shared<Miner>(id, session->ip);
        minersMap.Set(id, miner);
    }

    LogPrintf("Miner connected: %s@%s\n", id, session->ip);

    {
        addSession(session);
    }

    miner->heartbeat();

    auto jobReply = std::make_shared<JobReply>();
    auto sessionJob = *session->getJob(t.get());

    jobReply->job = sessionJob.toUniValue();
    jobReply->status = "OK";
    jobReply->id = id;

    return {jobReply, nullptr};
}

std::pair<std::shared_ptr<JobReplyData>, std::shared_ptr<ErrorReply>> StratumServer::handleGetJobRPC(std::shared_ptr<Session> session, const GetJobParams& params)
{
    std::shared_ptr<Miner> miner;
    {
        std::unique_lock<boost::shared_mutex> lock(minersMutex);
        auto result = minersMap.Get(params.id);
        auto retrievedMiner = result.first;
        auto found = result.second;
        if (found) {
            miner = retrievedMiner;
        }
    }

    if (!miner) {
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Unauthenticated";
        return {nullptr, error};
    }

    auto t = currentBlockTemplate();
    if (!t) {
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Job not ready";
        return {nullptr, error};
    }

    miner->heartbeat();
    auto jobData = session->getJob(t.get());
    return {jobData, nullptr};
}


std::pair<bool, bool> StratumServer::processShare(std::shared_ptr<Miner> miner, std::shared_ptr<Session> session, const SubmitParams& params)
{
    auto t = currentBlockTemplate();
    auto refreshnow = false;
    std::vector<uint8_t> shareBuff = t->blob_buffer;

    auto iid = session->getInstanceId();
    auto job = session->findJob(params.job_id);
    auto jb = t->nextBlob(job->getExtraNonce(), iid);

    if (iid.size() >= 3 &&
        t->reserved_offset + 7 <= shareBuff.size()) {
        std::copy(iid.begin(), iid.end(), shareBuff.begin() + t->reserved_offset + 4);
    } else {
        LogPrintf("Bad hash from miner %s@%s, instance id.\n", miner->id, miner->ip);
        miner->invalidShares++;
        return {false, refreshnow};
    }

    uint32_t extraNonceBE = htonl(job->getExtraNonce());

    if (t->reserved_offset + sizeof(extraNonceBE) <= shareBuff.size()) {
        std::memcpy(&shareBuff[t->reserved_offset], &extraNonceBE, sizeof(extraNonceBE));
    } else {
        LogPrintf("Bad hash from miner %s@%s, extra nonce from job.\n", miner->id, miner->ip);
        miner->invalidShares++;
        return {false, refreshnow};
    }

    std::vector<uint8_t> nonceBuff = util::hexDecode(params.nonce);
    if (39 + nonceBuff.size() <= shareBuff.size()) {
        std::copy(nonceBuff.begin(), nonceBuff.end(), shareBuff.begin() + 39);
    } else {
        LogPrintf("Bad hash from miner %s@%s, submitted nonce: %s.\n", miner->id, miner->ip, params.nonce);
        miner->invalidShares++;
        return {false, refreshnow};
    }

    std::vector<uint8_t> hashBytes;
    std::vector<uint8_t> convertedBlob = util::ConvertBlob(shareBuff);

    if (t->seed_hash.empty()) {
        // cn/r
        hashBytes = util::Hash(convertedBlob, false, static_cast<int>(t->height), "");
    } else {
        // rx/keva
        hashBytes = util::Hash(convertedBlob, false, static_cast<int>(t->height), t->seed_hash);
    }

    std::string computedHash = util::bytesToHexString(hashBytes);
    if (computedHash != params.result) {
        LogPrintf("Bad hash from miner %s@%s computed: %s submitted: %s.\n", miner->id, miner->ip, computedHash, params.result);
        miner->invalidShares++;
        return {false, refreshnow};
    }
    
    auto hashDiffPair = util::GetHashDifficulty(hashBytes);

    if (!hashDiffPair.second) {
        LogPrintf("Bad hash from miner %s@%s unable to get hash difficulty.\n", miner->id, miner->ip);
        miner->invalidShares++;
        return {false, refreshnow};
    }
    auto hashDiff = hashDiffPair.first;
    bool block = (hashDiff >= mp::uint256_t(t->difficulty));

    if (block) {
        try {
            auto submitResult = rcli->SubmitBlock(util::bytesToHexString(shareBuff));
            if (submitResult["status"].get_str() == "OK") {
                // Update block stats
                std::string blockFastHash = util::bytesToHexString(util::reverseBytes(util::FastHash(convertedBlob)));
                auto now = util::MakeTimestamp();
                // double ratio = static_cast<double>(roundShares) / static_cast<double>(t->difficulty);
                // std::lock_guard<std::mutex> lock(blocksMu);
                // blockStats[now] = blockEntry{ t->height, ratio, blockFastHash };
                
                miner->accepts++;
                miner->lastsubmissionat = now;
                LogPrintf("Block %s found at height %s by miner %s@%s!\n", blockFastHash, t->height, miner->id, miner->ip);

                refreshnow = true;
            } else {
                miner->rejects++;
                LogPrintf("Block rejected at height %s: %s / share: %s.\n", t->height, submitResult.get_str(), hashDiff);
                return {false, refreshnow};
            }
        } catch (const std::exception& e) {
            miner->rejects++;
            LogPrintf("Block rejected at height %s: %s / share: %s.\n", t->height, e.what(), hashDiff);
            return {false, refreshnow};
        }
    }
    if (hashDiff.convert_to<int64_t>() < session->ediff) {
        LogPrintf("Rejected low difficulty share of %s from miner %s@%s.\n", hashDiff, miner->id, miner->ip);
        miner->invalidShares++;
        return {false, refreshnow};
    }

    // roundShares += session->ediff;
    roundShares += hashDiff.convert_to<int64_t>();
    miner->validShares++;
    miner->storeShare(hashDiff.convert_to<int64_t>());
    // miner->storeShare(session->ediff);

    LogPrintf("Valid share with diff of %s from miner %s@%s.\n", hashDiff, miner->id, miner->ip);
    return {true, refreshnow};
}

int64_t StratumServer::getRoundShares()
{
    return roundShares;
}

Config* StratumServer::getConfig()
{
    return config;
}

std::pair<std::shared_ptr<StatusReply>, std::shared_ptr<ErrorReply>> StratumServer::handleSubmitRPC(std::shared_ptr<Session> session, const SubmitParams& params)
{
    std::shared_ptr<Miner> miner;
    {
        std::unique_lock<boost::shared_mutex> lock(minersMutex);
        auto result = minersMap.Get(params.id);
        auto retrievedMiner = result.first;
        auto found = result.second;
        if (found) {
            miner = retrievedMiner;
        }
    }

    if (!miner) {
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Unauthenticated";
        return {nullptr, error};
    }

    auto job = session->findJob(params.job_id);
    if (!job) {
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Invalid job id";
        return {nullptr, error};
    }

    static const std::regex noncePattern("^[0-9a-f]{8}$");
    if (!std::regex_match(params.nonce, noncePattern)) {
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Malformed nonce";
        return {nullptr, error};
    }

    std::string nonce = params.nonce;
    std::transform(nonce.begin(), nonce.end(), nonce.begin(), ::tolower);
    bool exists = job->submit(nonce);
    if (exists) {
        miner->invalidShares++;
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Duplicate share";
        return {nullptr, error};
    }

    auto t = currentBlockTemplate();
    if (!t || job->getHeight() != t->height) {
        LogPrintf("Stale share for height %s from %s@%s\n", t->height, miner->id, miner->ip);
        miner->staleShares++;
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Block expired";
        return {nullptr, error};
    }

    auto preply = processShare(miner, session, params);

    if (!preply.first) {
        auto error = std::make_shared<ErrorReply>();
        error->code = -1;
        error->message = "Low difficulty share";
        return {nullptr, error};
    }
    if (preply.second) {
        refreshBlockTemplate(true);
    }

    miner->heartbeat();
    auto status = std::make_shared<StatusReply>();
    status->status = "OK";

    return {status, nullptr};
}

std::shared_ptr<ErrorReply> StratumServer::handleUnknownRPC(const UniValue& req)
{
    LogPrintf("Unknown RPC method: %s\n", req.write());
    auto error = std::make_shared<ErrorReply>();
    error->code = -1;
    error->message = "Invalid method";
    return error;
}

void StratumServer::broadcastNewJobs()
{
    auto t = currentBlockTemplate();
    if (!t) {
        return;
    }

    std::unordered_set<std::shared_ptr<Session>> sessionsCopy;
    {
        std::lock_guard<std::mutex> lock(sessionsMutex);
        sessionsCopy = sessions;
    }

    LogPrintf("Broadcasting new jobs to %s miners\n", sessionsCopy.size());
    std::vector<std::thread> threads;

    for (auto& session : sessionsCopy) {
        threads.emplace_back([this, session, t]() {
            auto reply = *session->getJob(t.get());
            try {
                session->pushMessage("job", reply);
            }
            catch (const std::exception& e) {
                LogPrintf("Job transmit error to %s: %s\n", session->ip, e.what());
            } });
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

void StratumServer::refreshBlockTemplate(bool broadcast)
{
    bool newBlock = fetchBlockTemplate();
    if (newBlock && broadcast) {
        broadcastNewJobs();
    }
}

bool StratumServer::fetchBlockTemplate()
{
    auto rpcClient = rpc();
    if (!rpcClient) {
        LogPrintf("RPC Client is null.\n");
        return false;
    }

    GetBlockTemplateReply reply;
    try {
        reply = rpcClient->GetBlockTemplate(8, config->Address);
    } catch (const std::exception& e) {
        LogPrintf("Error while refreshing block template for stratum: %s\n", e.what());
        return false;
    }

    auto t = currentBlockTemplate();

    if (t) {
        if (t->height == reply.height) {
            // TODO: Check if mempool changed
            return false;
        }
    }

    auto newTemplate = std::make_shared<BlockTemplate>();
    newTemplate->difficulty = reply.difficulty;
    newTemplate->height = reply.height;
    newTemplate->prev_hash = reply.prev_hash;
    newTemplate->reserved_offset = reply.reserved_offset;
    newTemplate->seed_hash = reply.seed_hash;
    newTemplate->next_seed_hash = reply.next_seed_hash;
    newTemplate->blocktemplate_blob = reply.blocktemplate_blob;

    try {
        newTemplate->blob_buffer = util::hexDecode(reply.blocktemplate_blob);
    } catch (const std::exception& e) {
        LogPrintf("Failed to decode blob: %s\n", e.what());
        return false;
    }

    std::lock_guard<std::mutex> lock(blockTemplateMutex);
    blockTemplate = newTemplate;
    return true;
}

int StratumServer::getSessionCount()
{
    std::lock_guard<std::mutex> lock(sessionsMutex);
    return sessions.size();
}

std::shared_ptr<BlockTemplate> StratumServer::currentBlockTemplate()
{
    std::lock_guard<std::mutex> lock(blockTemplateMutex);
    return blockTemplate;
}

std::shared_ptr<RPCClient> StratumServer::rpc() const
{
    return rcli;
}

void StratumServer::Listen()
{
    try {
        for (auto& portConfig : config->Stratum.listen) {
            auto io_context = std::make_shared<boost::asio::io_context>();
            auto work_guard = boost::asio::make_work_guard(io_context);
            auto endpoint = std::make_shared<Endpoint>(&portConfig, *io_context);

            endpoint->Listen(this, &portConfig);

            std::thread thread([io_context]() { io_context->run(); });
            
            endpointIoServices.push_back(io_context);
            endpointWorks.push_back(std::make_shared<decltype(work_guard)>(std::move(work_guard)));
            endpointThreads.emplace_back(std::move(thread));
            endpoints.push_back(endpoint);
        }

        LogPrintf("StratumServer is now listening on all configured ports.\n");
    } catch (const std::exception& e) {
        LogPrintf("Error in StratumServer::Listen: %s\n", e.what());
    }
}

void StratumServer::addSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(sessionsMutex);
    sessions.insert(session);
}

void StratumServer::removeSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(sessionsMutex);
    sessions.erase(session);
}

MinersMap StratumServer::getMinersMap() const
{
    boost::shared_lock<boost::shared_mutex> lock(minersMutex);
    return minersMap;
}

} // namespace stratum