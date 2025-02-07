// job.h
#ifndef JOB_H
#define JOB_H

#include <mutex>
#include <string>
#include <unordered_set>

namespace stratum
{

class Job
{
public:
    Job(int64_t height, uint32_t extraNonce, const std::string& id);
    bool submit(const std::string& nonce);
    int64_t getHeight() const;
    uint32_t getExtraNonce() const;
    std::string getId() const;

private:
    int64_t height;
    uint32_t extraNonce;
    std::string id;
    std::unordered_set<std::string> submissions;
    mutable std::mutex mtx;
};

} // namespace stratum

#endif // JOB_H
