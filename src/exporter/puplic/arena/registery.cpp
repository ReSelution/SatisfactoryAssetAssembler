//
// Created by oleub on 22.02.26.
//

#include "registery.hpp"

#include <assert.h>

namespace SAP {
  void Registry::init(size_t bufferSize) {
    if (!initialized) {
      backingBuffer = std::make_unique_for_overwrite<uint8_t[]>(bufferSize);
      pool = std::make_unique<std::pmr::monotonic_buffer_resource>(backingBuffer.get(), bufferSize);
      registry[static_cast<uint32_t>(ExportType::Recipe)].reserve(1000);
      initialized = true;
    }
  }

  void Registry::reset() {
    pool->release();
  }

  void *Registry::internal_allocate(size_t size) {
    assert(initialized && "Registry not initialized");
    while (m_allocLock.test_and_set(std::memory_order_acquire)) {
    }
    void *ptr = pool->allocate(size);
    m_allocLock.clear(std::memory_order_release);
    return ptr;
  }
} // SAP
