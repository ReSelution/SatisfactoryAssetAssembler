//
// Created by oleub on 02.02.26.
//

#pragma once
#include <unordered_map>
#if defined(__x86_64__) || defined(_M_X64)
  #include <immintrin.h>
#endif

template<typename K, typename V, size_t ShardCount = 64>
class ShardedCache {

  struct IdentityHash {
    using is_transparent = void;

    size_t operator()(size_t key) const noexcept {
      return key;
    }
  };
  struct Shard {
    std::atomic_flag lock = ATOMIC_FLAG_INIT;
    std::unordered_map<K, V, IdentityHash> data;

    void acquire() {
      while (lock.test_and_set(std::memory_order_acquire)) {
#if defined(__x86_64__)
        _mm_pause();
#endif
      }
    }

    void release() {
      lock.clear(std::memory_order_release);
    }
  };
  std::array<Shard, ShardCount> shards;

  Shard &getShard(const K &key) { return shards[key % ShardCount]; }

public:

  void reserve(size_t size) {
    for (Shard &shard: shards) {
      shard.acquire();
      shard.data.reserve(size/ShardCount);
      shard.release();
    }
  }
  void reset() {
    for (Shard &shard: shards) {
      shard.acquire();
      for (auto &[hash, weakParser]: shard.data) {
        if (auto parser = weakParser.lock()) {
          parser->reset();
        }
      }
      shard.release();
      shard.data.clear();
    }
  }

  ~ShardedCache() { reset(); }

  V get(const K &key) {
    auto &shard = getShard(key);
    shard.acquire();
    auto it = shard.data.find(key);
    shard.release();
    return (it != shard.data.end()) ? it->second : V{};
  }

  void insert(const K &key, V value) {
    auto &shard = getShard(key);
    shard.acquire();
    shard.data[key] = std::move(value);
    shard.release();
  }
};
