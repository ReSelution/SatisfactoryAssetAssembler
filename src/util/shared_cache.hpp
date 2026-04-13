//
// Created by oleub on 02.02.26.
//

#pragma once

#include "threading/chaos_threading.hpp"
#include "threading/chaos_spin_lock.hpp"

#include "hash/hash.hpp"
#include <ankerl/unordered_dense.h>


LOG_ALIAS(chacheLOG, "SAA", "CACHE")

template<typename K, typename V, size_t ShardCount = 64>
class ShardedCache {


  struct alignas(64) Shard {
    SC::ChaosSpinLock lock;
    using denseMap = ankerl::unordered_dense::map<uint64_t, std::weak_ptr<V>, SC::IdentityHash>;

    denseMap data{};

    void acquire() { lock.lock(); }

    void release() { lock.unlock(); }
  };

  std::array<Shard, ShardCount> shards;

  Shard &getShard(const K &key) { return shards[key % ShardCount]; }

public:
  void reserve(size_t size) {
    for (Shard &shard: shards) {
      shard.acquire();
      shard.data.reserve(size / ShardCount);
      shard.release();
    }
  }

  void reset() {
    size_t count = 0;
    auto t = chacheLOG::time("Reset {1} in {0}", count);

    std::atomic atomicCount{count};
    std::vector<Shard *> shardPointers;

    shardPointers.reserve(shards.size());
    for (auto &shard: shards) {
      shardPointers.push_back(&shard);
    }


    auto futters = SC::ChaosThreading::enqueueBatch<SC::ChaosThreading::Priority::Normal>( shardPointers.begin(),
                                                    shardPointers.end(),
                                                    [&atomicCount, this](int id, Shard *shard) {
                                                      shard->acquire();
                                                      size_t counter = 0;
                                                      for (auto &[hash, weakParser]: shard->data) {
                                                        if (auto parser = weakParser.lock()) {
                                                          parser->safeReset();
                                                          counter++;
                                                        }
                                                      }
                                                      shard->release();
                                                      shard->data.clear();
                                                      atomicCount.fetch_add(counter);
                                                    }

    );
    for (auto &fut: futters) {
      fut.wait();
    }
    count = atomicCount.load();

    // SC::ChaosThreading::wait_until_finished();
    // for (Shard &shard : shards) {
    //   shard.acquire();
    //   for (auto &[hash, weakParser] : shard.data) {
    //     if (auto parser = weakParser.lock()) {
    //       parser->safeReset();
    //       count++;
    //     }
    //   }
    //   shard.release();
    //   shard.data.clear();
    // }
  }

  ~ShardedCache() { reset(); }


  std::shared_ptr<V> getLocked(const K &key) {
    auto &shard = getShard(key);
    shard.acquire();

    auto it = shard.data.find(key);
    if (it != shard.data.end()) {
      auto wPtr = it->second;
      shard.release();
      return std::move(wPtr.lock());
    }

    shard.release();
    return nullptr;
  }

  void insert(const K &key, std::shared_ptr<V> value) {
    auto &shard = getShard(key);
    shard.acquire();
    shard.data[key] = std::move(value);
    shard.release();
  }
};
