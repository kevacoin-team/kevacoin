#ifndef STRATUM_SERVER_H
#define STRATUM_SERVER_H

#include "blocktemplate.h"
#include "config.h"
#include "miner.h"
#include "miners_map.h"
#include "rpc.h"
#include "session.h"
#include "structures.h"
#include <boost/asio.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_set>
#include <vector>
namespace mp = boost::multiprecision;

namespace stratum
{

class Session;
class Endpoint;

class StratumServer : public std::enable_shared_from_this<Session>
{
public:
    StratumServer(Config* config);
    ~StratumServer();

    std::pair<std::shared_ptr<JobReply>, std::shared_ptr<ErrorReply>> handleLoginRPC(std::shared_ptr<Session> session, const LoginParams& params);
    std::pair<std::shared_ptr<JobReplyData>, std::shared_ptr<ErrorReply>> handleGetJobRPC(std::shared_ptr<Session> session, const GetJobParams& params);
    std::pair<std::shared_ptr<StatusReply>, std::shared_ptr<ErrorReply>> handleSubmitRPC(std::shared_ptr<Session> session, const SubmitParams& params);
    std::shared_ptr<ErrorReply> handleUnknownRPC(const nlohmann::json& req);

    void broadcastNewJobs();
    void Listen();
    void refreshBlockTemplate(bool broadcast);
    bool fetchBlockTemplate();

    std::shared_ptr<BlockTemplate> currentBlockTemplate();
    int64_t getRoundShares();

    std::shared_ptr<RPCClient> rpc() const;
    Config* getConfig();
    void addSession(std::shared_ptr<Session> session);
    void removeSession(std::shared_ptr<Session> session);
    MinersMap getMinersMap() const;
    int getSessionCount();

private:
    Config* config;
    std::map<int64_t, blockEntry> blockStats;
    int64_t roundShares;
    std::shared_ptr<RPCClient> rcli;
    MinersMap minersMap;
    mutable boost::shared_mutex minersMutex;

    std::pair<bool, bool> processShare(std::shared_ptr<Miner> miner, std::shared_ptr<Session> session, const SubmitParams& params);
    std::shared_ptr<BlockTemplate> blockTemplate;
    std::mutex blockTemplateMutex;
    // std::mutex blocksMu;
    std::vector<std::shared_ptr<boost::asio::io_service>> endpointIoServices;
    std::vector<std::shared_ptr<boost::asio::io_service::work>> endpointWorks;
    std::vector<std::thread> endpointThreads;
    std::vector<std::shared_ptr<Endpoint>> endpoints;

    std::unordered_set<std::shared_ptr<Session>> sessions;
    mutable std::mutex sessionsMutex;
    std::thread refreshThread;
    bool stopFlag;
};

} // namespace stratum

#endif // STRATUM_SERVER_H
