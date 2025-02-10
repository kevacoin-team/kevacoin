#ifndef SESSION_H
#define SESSION_H

#include "blocktemplate.h"
#include "job.h"
#include "stratum_server.h"
#include "structures.h"
#include "univalue.h"
#include <array>
#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace stratum
{

class Endpoint;
class StratumServer;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& ip, const int64_t edfiff);
    ~Session();

    void start(StratumServer* server);
    std::pair<std::string, std::string> extractWorkerId(const std::string& loginWorkerPair);
    std::shared_ptr<JobReplyData> getJob(BlockTemplate* t);
    std::shared_ptr<Job> findJob(const std::string& jobId);
    void pushMessage(const std::string& method, const JobReplyData reply);
    bool handleMessage(StratumServer* server, const JSONRpcReq& req);
    const std::array<uint8_t, 3>& getInstanceId() const;
    const int64_t ediff;
    std::string ip;

private:
    void do_read();
    void handle_line(const std::string& line);
    bool send_json(const UniValue& j);
    void pushJob(std::shared_ptr<Job> job);

    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    boost::asio::streambuf buffer;
    mutable std::mutex jobMtx;
    std::vector<std::shared_ptr<Job>> validJobs;
    StratumServer* server_;
    std::atomic<uint32_t> extraNonce{0};
    std::atomic<uint64_t> jobSequence{0};
    std::array<uint8_t, 3> instanceId;
};

} // namespace stratum

#endif // SESSION_H
