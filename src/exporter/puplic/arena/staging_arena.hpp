//
// Created by oleub on 22.02.26.
//

#pragma once
#include <memory>
#include <memory_resource>

namespace SAP {
  class StagingArena {
    public:


    static void init(size_t initialSize = 1024*1024);

    private:
    static void* internal_allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
      while (allocLock.test_and_set(std::memory_order_acquire)) {}
      m_usedBytes += size;
      auto ret =  m_pool->allocate(size, alignment);
      allocLock.clear(std::memory_order_release);
      return ret;
    }

    static inline bool m_initialized = false;
    static inline size_t m_capacity = 16 * 1024;
    static inline  size_t m_usedBytes = 0;
    static inline std::unique_ptr<uint8_t[]> m_backingBuffer;
    static inline std::unique_ptr<std::pmr::monotonic_buffer_resource> m_pool;

    static inline std::atomic_flag allocLock = ATOMIC_FLAG_INIT;
  };
} // SAP