//
// Created by oleub on 11.02.26.
//

#pragma once
#include <atomic>
#include <cassert>
#include <memory>
#include <thread>
#include <vector>
#include "heap_storage.hpp"

namespace SAP {

  class HeapPool {
    static inline std::vector<std::unique_ptr<HeapStorage>> m_storage;
    static inline std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
    static inline bool m_initialized = false;
    static inline size_t bufferSize = 16 * 1024;

  public:

    static size_t getBufferSize() { return bufferSize; }

    static void setBufferSize(size_t newBufferSize) { bufferSize = newBufferSize; }

    static void init(size_t poolSize, size_t initBufferSize = 16 * 1024) {
      if (m_initialized)
        return;
      bufferSize = initBufferSize;
      m_storage.reserve(poolSize);
      for (size_t i = 0; i < poolSize; ++i) {
        m_storage.push_back(std::make_unique<HeapStorage>(bufferSize));
      }
      m_initialized = true;
    }

    static void reset() {
      m_initialized = false;
      m_storage.clear();
    }

    static std::unique_ptr<HeapStorage> acquire() {
      assert(m_initialized && "HeapPool not initialized! Call init() in main.");

      while (m_lock.test_and_set(std::memory_order_acquire)) {}

      if (m_storage.empty()) {
        m_lock.clear(std::memory_order_release);
        return std::make_unique<HeapStorage>(bufferSize);
      }

      auto s = std::move(m_storage.back());
      m_storage.pop_back();
      m_lock.clear(std::memory_order_release);
      return s;
    }

    static void release(std::unique_ptr<HeapStorage> &s) {
      if (!s)
        return;
      s->reset();

      while (m_lock.test_and_set(std::memory_order_acquire)) {}

      m_storage.push_back(std::move(s));
      m_lock.clear(std::memory_order_release);
    }
  };

} // namespace SAP
