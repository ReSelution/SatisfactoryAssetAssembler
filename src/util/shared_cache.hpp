//
// Created by oleub on 02.02.26.
//

#pragma once
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

template<typename K, typename V, size_t ShardCount = 64>
class ShardedCache {
  struct Shard {
    std::shared_mutex mutex;
    std::unordered_map<K, V> data;
  };
  std::array<Shard, ShardCount> shards;

  Shard &getShard(const K &key) { return shards[key % ShardCount]; }

public:
  void reset() {
    for (Shard &shard: shards) {
      std::unique_lock lock(shard.mutex);
      for (auto &[hash, weakParser]: shard.data) {
        if (auto parser = weakParser.lock()) {
          parser->reset();
        }
      }
      shard.data.clear();
    }
  }

  ~ShardedCache() { reset(); }

  V get(const K &key) {
    auto &shard = getShard(key);
    std::shared_lock lock(shard.mutex);
    auto it = shard.data.find(key);
    return (it != shard.data.end()) ? it->second : V{};
  }

  void insert(const K &key, V value) {
    auto &shard = getShard(key);
    std::unique_lock lock(shard.mutex);
    shard.data[key] = std::move(value);
  }
};
