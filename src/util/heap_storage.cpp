//
// Created by oleub on 10.02.26.
//

#include "heap_storage.hpp"

#include "heap_pool.hpp"

namespace SAA {
  HeapStorage::HeapStorage(size_t initialSize) :
      m_capacity(initialSize), m_backingBuffer(std::make_unique_for_overwrite<uint8_t[]>(initialSize)) {
    new (&m_pool) std::pmr::monotonic_buffer_resource(m_backingBuffer.get(), m_capacity);
  }
} // namespace SAP
