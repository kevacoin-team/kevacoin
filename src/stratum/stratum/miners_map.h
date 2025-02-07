// miners_map.h
#ifndef MINERS_MAP_H
#define MINERS_MAP_H

#include "miner.h"
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace stratum
{
class Miner;
}

namespace stratum
{
struct Tuple {
    std::string Key;
    Miner* Val;
};

class MinersMapShared
{
public:
    MinersMapShared() : items() {}

    void Set(const std::string& key, std::shared_ptr<Miner> value)
    {
        std::unique_lock<boost::shared_mutex> lock(mtx);
        items[key] = value;
    }

    std::pair<std::shared_ptr<Miner>, bool> Get(const std::string& key) const
    {
        std::unique_lock<boost::shared_mutex> lock(mtx);
        auto it = items.find(key);
        if (it != items.end()) {
            return {it->second, true};
        }
        return {nullptr, false};
    }

    void Remove(const std::string& key)
    {
        std::unique_lock<boost::shared_mutex> lock(mtx);
        items.erase(key);
    }

    bool Has(const std::string& key) const
    {
        std::unique_lock<boost::shared_mutex> lock(mtx);
        return items.find(key) != items.end();
    }

    size_t Count() const
    {
        std::unique_lock<boost::shared_mutex> lock(mtx);
        return items.size();
    }

    std::vector<Tuple> GetAll() const
    {
        std::unique_lock<boost::shared_mutex> lock(mtx);
        std::vector<Tuple> tuples;
        tuples.reserve(items.size());
        for (const auto& pair : items) {
            tuples.emplace_back(Tuple{pair.first, pair.second.get()});
        }
        return tuples;
    }

private:
    mutable boost::shared_mutex mtx;
    std::unordered_map<std::string, std::shared_ptr<Miner>> items;
};

class MinersMap
{
public:
    static constexpr size_t SHARD_COUNT = 32;

    MinersMap() : shards(SHARD_COUNT)
    {
        for (size_t i = 0; i < SHARD_COUNT; ++i) {
            shards[i] = std::make_shared<MinersMapShared>();
        }
    }

    void Set(const std::string& key, std::shared_ptr<Miner> value)
    {
        GetShard(key)->Set(key, value);
    }

    std::pair<std::shared_ptr<Miner>, bool> Get(const std::string& key) const
    {
        return GetShard(key)->Get(key);
    }

    void Remove(const std::string& key)
    {
        GetShard(key)->Remove(key);
    }

    bool Has(const std::string& key) const
    {
        return GetShard(key)->Has(key);
    }

    size_t Count() const
    {
        size_t count = 0;
        for (const auto& shard : shards) {
            count += shard->Count();
        }
        return count;
    }

    bool IsEmpty() const
    {
        return Count() == 0;
    }

    std::vector<Tuple> Iter() const
    {
        std::vector<Tuple> allTuples;
        for (const auto& shard : shards) {
            auto shardTuples = shard->GetAll();
            allTuples.insert(allTuples.end(), shardTuples.begin(), shardTuples.end());
        }
        return allTuples;
    }

private:
    // Retrieves the shard for the given key
    std::shared_ptr<MinersMapShared> GetShard(const std::string& key) const
    {
        std::hash<std::string> hasher;
        size_t hash = hasher(key);
        size_t shardIdx = hash % SHARD_COUNT;
        return shards[shardIdx];
    }

    std::vector<std::shared_ptr<MinersMapShared>> shards;
};

} // namespace stratum

#endif // MINERS_MAP_H
