//
// Created by oleub on 10.02.26.
//

#pragma once
#include <cstring>
#include <memory>
#include <memory_resource>
#include <span>
#include <string_view>

namespace SAP {
  template<size_t bufferSize = 1024>
  class HeapStorage {
  public:
    HeapStorage();

    std::string_view utf16ToUtf8Arena(const std::span<const char16_t>& input) {
      if (input.empty()) return "";


      size_t maxLen = input.size() * 3;
      char* target = static_cast<char*>(m_pool->allocate(maxLen + 1));

      char* curr = target;
      for (uint16_t cp : input) {
        if (cp < 0x80) {
          *curr++ = static_cast<char>(cp);
        } else if (cp < 0x800) {
          *curr++ = static_cast<char>(0xC0 | (cp >> 6));
          *curr++ = static_cast<char>(0x80 | (cp & 0x3F));
        } else {
          *curr++ = static_cast<char>(0xE0 | (cp >> 12));
          *curr++ = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
          *curr++ = static_cast<char>(0x80 | (cp & 0x3F));
        }
      }
      *curr = '\0'; // Null-Terminator

      size_t finalLen = curr - target;
      return {target, finalLen};
    }

    std::string_view persistString(std::string_view str) {
      char *mem = static_cast<char *>(m_pool->allocate(str.size() + 1));
      std::memcpy(mem, str.data(), str.size());
      mem[str.size()] = '\0';
      return {mem, str.size()};
    }

    template<typename T, typename... Args>
    T *make(Args &&... args) {
      void *mem = m_pool->allocate(sizeof(T), alignof(T));
      return new(mem) T(std::forward<Args>(args)...);
    };

    [[nodiscard]] void* allocateRaw(size_t size, size_t alignment = alignof(std::max_align_t)) {
      return m_pool->allocate(size, alignment);
    }
  private:
    std::unique_ptr<std::pmr::monotonic_buffer_resource> m_pool;

  };

  template<size_t bufferSize>
  HeapStorage<bufferSize>::HeapStorage() : m_pool(std::make_unique<std::pmr::monotonic_buffer_resource>(bufferSize)) {}
} // SAP
