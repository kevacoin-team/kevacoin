// miner.cpp
#include "miner.h"
#include "stratum/util/util.h"
#include <chrono>

namespace stratum
{

Miner::Miner(const std::string& id, const std::string& ip)
    : id(id), ip(ip), accepts(0), rejects(0), validShares(0), staleShares(0), invalidShares(0), lastBeat(0) {}

void Miner::heartbeat()
{
    auto now = util::MakeTimestamp();
    std::lock_guard<std::mutex> lock(lastBeatMutex);
    lastBeat = now;
}

uint64_t Miner::getLastBeat()
{
    std::lock_guard<std::mutex> lock(lastBeatMutex);
    return lastBeat;
}

double Miner::hashrate(std::chrono::milliseconds estimationWindow)
{
    auto now = util::MakeTimestamp();
    int64_t totalShares = 0.0;
    int64_t window = estimationWindow.count();
    int64_t cou = 0;
    auto boundary = now - getLastBeat();
    auto z = window / 60 * 0.58;
    if (boundary > window) {
        boundary = window;
    }

    {
        std::lock_guard<std::mutex> lock(sharesMutex);

        for (auto it : shares) {
            if (it.first < now - 86400000)
            {
                shares.erase(it.first);
            }
            else
            {
                if (it.first >= now - window) {
                    totalShares += it.second;
                    cou += 1;
                }
            }
        }
    }
    
    return ((static_cast<double>(totalShares) / cou) / static_cast<double>(window)) * z;
}

uint64_t Miner::average_share_difficulty()
{
    uint64_t totalShares = 0;
    uint64_t counter = 0;

    {
        std::lock_guard<std::mutex> lock(sharesMutex);

        for (auto it : shares) {
            totalShares += it.second;
            counter += 1;
        }
    }
    if (counter > 0 && totalShares > 0) {
        return totalShares / counter;
    } else {
        return 0;
    }
}

void Miner::storeShare(int64_t diff)
{
    auto now = util::MakeTimestamp();
    {
        std::lock_guard<std::mutex> lock(sharesMutex);
        shares[now] += diff;
    }
}

} // namespace stratum
