// job.cpp
#include "job.h"

namespace stratum
{

Job::Job(int64_t height, uint32_t extraNonce, const std::string& id)
    : height(height), extraNonce(extraNonce), id(id) {}

bool Job::submit(const std::string& nonce)
{
    std::lock_guard<std::mutex> lock(mtx);
    return !submissions.emplace(nonce).second;
}

int64_t Job::getHeight() const
{
    return height;
}

uint32_t Job::getExtraNonce() const
{
    return extraNonce;
}

std::string Job::getId() const
{
    return id;
}

} // namespace stratum
