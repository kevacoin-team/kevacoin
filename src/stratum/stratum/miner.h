// miner.h
#ifndef MINER_H
#define MINER_H

#include <map>
#include <mutex>
#include <string>

namespace stratum
{

class Miner
{
public:
    Miner(const std::string& id, const std::string& ip);
    void heartbeat();
    uint64_t getLastBeat();
    double hashrate(std::chrono::milliseconds estimationWindow);
    uint64_t average_share_difficulty();
    void storeShare(int64_t diff);

    uint64_t accepts;
    uint64_t rejects;
    uint64_t validShares;
    uint64_t staleShares;
    uint64_t invalidShares;

    uint64_t lastsubmissionat;

    std::string id;
    std::string ip;

private:
    uint64_t lastBeat;
    std::mutex lastBeatMutex;
    std::map<uint64_t, uint64_t> shares;
    std::mutex sharesMutex;
};

} // namespace stratum

#endif // MINER_H
