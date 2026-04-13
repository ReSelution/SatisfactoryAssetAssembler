//
// Created by oleub on 22.02.26.
//

#include "registery.hpp"

#include <assert.h>

namespace SAA {
  void Registry::init(size_t bufferSize) {
    if (!initialized) {
      backingBuffer = std::make_unique_for_overwrite<uint8_t[]>(bufferSize);
      pool = std::make_unique<SC::ChaosMonotonicResource>(backingBuffer.get(), bufferSize);
      registry[static_cast<uint32_t>(ExportType::Recipe)].reserve(1000);
      initialized = true;
    }
  }

  void Registry::reset() {
    pool->release();
  }

  void *Registry::internal_allocate(size_t size) {
    assert(initialized && "Registry not initialized");
    void *ptr = pool->allocate(size);
    return ptr;
  }
} // SAP
