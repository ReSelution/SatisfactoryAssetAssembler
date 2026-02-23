//
// Created by oleub on 22.02.26.
//

#include "staging_arena.hpp"

namespace SAP {
  void StagingArena::init(size_t initialSize ) {
    if (m_initialized)
      return;
    m_initialized = true;
    m_capacity = initialSize;
    m_backingBuffer= std::make_unique_for_overwrite<uint8_t[]>(m_capacity);
    m_pool = std::make_unique<std::pmr::monotonic_buffer_resource>(m_backingBuffer.get(), m_capacity);
  }
} // SAP