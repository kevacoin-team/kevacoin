// session.cpp
#include "session.h"
#include "stratum/util/util.h"
#include "structures.h"
#include <nlohmann/json.hpp>
#include <util.h>


namespace stratum
{

Session::Session(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& ip, const int64_t ediff)
    : socket_(socket), ip(ip), ediff(ediff), buffer(), validJobs()
{
    if (!util::random_bytes(instanceId.data(), instanceId.size())) {
        throw std::runtime_error("Can't seed with random bytes");
    }
}

Session::~Session()
{
    if (socket_->is_open()) {
        socket_->close();
    }
}

std::shared_ptr<JobReplyData> Session::getJob(BlockTemplate* t)
{
    int64_t height = t->height;
    uint32_t aextraNonce = extraNonce.fetch_add(1, std::memory_order_relaxed);
    uint64_t jobId = jobSequence.fetch_add(1, std::memory_order_relaxed);

    std::string blob = t->nextBlob(aextraNonce, instanceId);
    std::string jobIdStr = std::to_string(jobId);

    auto job = std::make_shared<Job>(height, aextraNonce, jobIdStr);
    pushJob(job);

    // Extract major version from blob (first byte)
    if (blob.size() < 2) {
        LogPrintf("Blob too short to extract major version.\n");
        return std::make_shared<JobReplyData>();
    }
    int64_t majorVersion = 0;
    try {
        majorVersion = std::stoi(blob.substr(0, 2), nullptr, 16);
    } catch (const std::exception& e) {
        LogPrintf("ailed to get major version: %s\n", e.what());
    }

    std::string algo;
    if (majorVersion == 10) {
        algo = "cn/r";
    } else if (majorVersion == 12) {
        algo = "rx/keva";
    }

    std::string targetHex = util::GetTargetHex(ediff);

    auto reply = std::make_shared<JobReplyData>();
    reply->job_id = jobIdStr;
    reply->blob = blob;
    reply->target = targetHex;
    reply->height = t->height;
    reply->algo = algo;
    reply->seed_hash = t->seed_hash;
    reply->next_seed_hash = t->next_seed_hash;

    return reply;
}

void Session::pushJob(std::shared_ptr<Job> job)
{
    std::lock_guard<std::mutex> lock(jobMtx);
    validJobs.push_back(job);

    if (validJobs.size() > 4) {
        validJobs.erase(validJobs.begin()); // Keep only the last 4 jobs
    }
}

std::shared_ptr<Job> Session::findJob(const std::string& jobId)
{
    std::lock_guard<std::mutex> lock(jobMtx);
    for (const auto& job : validJobs) {
        if (job->getId() == jobId) {
            return job;
        }
    }
    return nullptr;
}

void Session::pushMessage(const std::string& method, const JobReplyData reply)
{
    JSONRpcPush result;
    result.method = method;
    result.params = reply;

    send_json(result);
}


std::pair<std::string, std::string> Session::extractWorkerId(const std::string& loginWorkerPair)
{
    size_t dotPos = loginWorkerPair.find('.');
    if (dotPos != std::string::npos) {
        return {loginWorkerPair.substr(0, dotPos), loginWorkerPair.substr(dotPos + 1)};
    }
    return {loginWorkerPair, "0"}; // Default worker ID is "0"
}

void Session::start(StratumServer* server)
{
    // Start asynchronous read
    this->server_ = server;
    do_read();
}

void Session::handle_line(const std::string& line)
{
    try {
        auto json_req = nlohmann::json::parse(line);
        JSONRpcReq req = json_req.get<JSONRpcReq>();
        bool continue_session = handleMessage(server_, req);
        if (!continue_session) {
            // Close the connection if necessary
            socket_->close();
        }
    } catch (const std::exception& e) {
        LogPrintf("Malformed request from %s:%s.\n", ip, e.what());
        socket_->close();
    }
}

const std::array<uint8_t, 3>& Session::getInstanceId() const
{
    return instanceId;
}

void Session::do_read()
{
    auto self(shared_from_this());
    boost::asio::async_read_until(*socket_, buffer, "\n",
        [this, self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream is(&buffer);
                std::string line;
                std::getline(is, line);

                if (!line.empty()) {
                    handle_line(line);
                }

                do_read();
            } else {
                LogPrintf("Client disconnected: %s\n", ip);
                server_->removeSession(shared_from_this());
            }
        });
}

bool Session::handleMessage(StratumServer* server, const JSONRpcReq& req)
{
    // Handle RPC methods
    if (req.method == "login") {
        LoginParams params;
        try {
            params = req.params.get<LoginParams>();
        } catch (const std::exception& e) {
            LogPrintf("Unable to parse params: %s\n", e.what());
            return false;
        }
        auto result = server_->handleLoginRPC(shared_from_this(), params);
        if (result.second) {
            JSONRpcResp eresp;
            eresp.id = req.id;
            eresp.version = "2.0";
            eresp.error = *result.second;
            send_json(eresp);
            return false; // Drop connection if necessary
        }
        JSONRpcResp resp;
        resp.id = req.id;
        resp.version = "2.0";
        resp.result = *result.first;
        send_json(resp);
    } else if (req.method == "getjob") {
        GetJobParams params;
        try {
            params = req.params.get<GetJobParams>();
        } catch (const std::exception& e) {
            LogPrintf("Unable to parse params: %s\n", e.what());
            return false;
        }
        auto result = server_->handleGetJobRPC(shared_from_this(), params);
        if (result.second) {
            JSONRpcResp eresp;
            eresp.id = req.id;
            eresp.version = "2.0";
            eresp.error = *result.second;
            send_json(eresp);
            return false;
        }
        JSONRpcResp resp;
        resp.id = req.id;
        resp.version = "2.0";
        resp.result = *result.first;
        send_json(resp);
    } else if (req.method == "submit") {
        SubmitParams params;
        try {
            params = req.params.get<SubmitParams>();
        } catch (const std::exception& e) {
            LogPrintf("Unable to parse params: %s\n", e.what());
            return false;
        }
        auto result = server_->handleSubmitRPC(shared_from_this(), params);
        if (result.second) {
            JSONRpcResp eresp;
            eresp.id = req.id;
            eresp.version = "2.0";
            eresp.error = *result.second;
            send_json(eresp);
            // return false;
        } else {
            JSONRpcResp resp;
            resp.id = req.id;
            resp.version = "2.0";
            resp.result = *result.first;
            send_json(resp);
        }

    } else if (req.method == "keepalived") {
        StatusReply status = {"KEEPALIVED"};
        JSONRpcResp resp;
        resp.id = req.id;
        resp.version = "2.0";
        resp.result = status;
        send_json(resp);
    } else {
        auto errReply = server_->handleUnknownRPC(req);
        JSONRpcResp eresp;
        eresp.id = req.id;
        eresp.version = "2.0";
        eresp.error = *errReply;
        send_json(eresp);
        return false;
    }

    return true;
}

bool Session::send_json(const nlohmann::json& j)
{
    try {
        std::string serialized = j.dump() + "\n";
        boost::asio::write(*socket_, boost::asio::buffer(serialized));
        return true;
    } catch (const std::exception& e) {
        LogPrintf("Failed to send JSON: %s\n", e.what());
        return false;
    }
}

} // namespace stratum
