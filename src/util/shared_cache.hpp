//
// Created by oleub on 02.02.26.
//

#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "sc/hash/hash.hpp"
#include "sc/logger/logger.hpp"
#include "sc/threading/spin_lock.hpp"
#include "sc/threading/threading.hpp"

using chacheLOG = sc::Logger<"SAA", "CACHE">;

template<typename K, typename V, size_t ShardCount = 64>
class ShardedCache {


  // How many values a shard holds on to by itself, see keepAlive().
  static constexpr size_t KEEP_ALIVE = 16;

  struct alignas(64) Shard {
    sc::SpinLock lock;
    using denseMap = ankerl::unordered_dense::map<uint64_t, std::weak_ptr<V>, sc::TransparentHash>;

    denseMap data{};

    // The values handed to keepAlive() last, see there.
    std::array<std::shared_ptr<V>, KEEP_ALIVE> recent{};
    size_t nextRecent = 0;

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
    // count muss als lvalue an time() gehen: nur lvalues werden erst beim Loggen gelesen, ein Temporary wie
    // atomicCount.load() wuerde sofort kopiert und immer 0 ausgeben.
    size_t count = 0;
    auto t       = chacheLOG::time("Reset {1} in {0}", count);

    std::atomic<size_t> atomicCount{0};
    std::vector<Shard *> shardPointers;

    shardPointers.reserve(shards.size());
    for (auto &shard: shards) {
      shardPointers.push_back(&shard);
    }

    auto fut = sc::threading::enqueueBatch(std::move(shardPointers), [&atomicCount](Shard *shard) {
      shard->acquire();
      size_t counter = 0;
      for (auto &[hash, weakParser]: shard->data) {
        if (auto parser = weakParser.lock()) {
          parser->safeReset();
          counter++;
        }
      }
      shard->release();
      atomicCount.fetch_add(counter);
    });
    fut.wait();
    count = atomicCount.load();

    // sc::threading::waitUntilFinished();
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
      return wPtr.lock();
    }

    shard.release();
    return nullptr;
  }

  // Returns the live entry for key. If there is none, value is stored and returned. Check and insert happen under
  // one lock, so two threads that ask for the same key at the same time end up with the same object.
  std::shared_ptr<V> getOrInsert(const K &key, std::shared_ptr<V> value) {
    auto &shard = getShard(key);
    shard.acquire();
    auto [it, inserted] = shard.data.try_emplace(key, value);
    if (!inserted) {
      if (auto live = it->second.lock()) {
        shard.release();
        return live;
      }
      it->second = value;
    }
    shard.release();
    return value;
  }

  // Holds on to value until KEEP_ALIVE other values of its shard were handed in after it. Without this a value
  // only lives as long as somebody outside holds it, and one that is asked for again and again by holders that
  // do not overlap is made anew every time.
  void keepAlive(const K &key, std::shared_ptr<V> value) {
    auto &shard = getShard(key);
    shard.acquire();
    for (const auto &kept: shard.recent) {
      if (kept == value) {
        shard.release();
        return;
      }
    }
    std::swap(shard.recent[shard.nextRecent], value);
    shard.nextRecent = (shard.nextRecent + 1) % KEEP_ALIVE;
    shard.release();
    // The value that fell out is released here, outside the lock: it may be the last holder.
  }

  void insert(const K &key, std::shared_ptr<V> value) {
    auto &shard = getShard(key);
    shard.acquire();
    shard.data[key] = std::move(value);
    shard.release();
  }
};
