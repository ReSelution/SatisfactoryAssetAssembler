//
// Created by oleub on 10.02.26.
//

#pragma once
#include <cstring>
#include <memory>
#include <memory_resource>
#include <span>
#include <string_view>

namespace SAA {
  class HeapStorage {

    struct CleanupNode {
      void (*destroyer)(void*);
      void* object;
      CleanupNode* next;
    };
  public:
    HeapStorage(size_t initialSize = 8 * 1024);
    std::pmr::memory_resource* memoryResource() {
      return &m_pool;
    }
    std::string_view utf16ToUtf8Arena(const std::span<const char16_t> &input) {
      if (input.empty())
        return "";

      size_t maxLen = input.size() * 3;
      char *target = static_cast<char *>(internal_allocate(maxLen + 1));

      char *curr = target;
      for (uint16_t cp: input) {
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
      char *mem = static_cast<char *>(internal_allocate(str.size() + 1));
      std::memcpy(mem, str.data(), str.size());
      mem[str.size()] = '\0';
      return {mem, str.size()};
    }

    template<typename T, typename... Args>
    T *make(Args &&...args) {
      void *mem =internal_allocate(sizeof(T), alignof(T));
      T *obj = new (mem) T(std::forward<Args>(args)...);

      if constexpr (!std::is_trivially_destructible_v<T>) {
        void* nodeMem = internal_allocate(sizeof(CleanupNode), alignof(CleanupNode));
        CleanupNode* node = new (nodeMem) CleanupNode{
          .destroyer = [](void* p) { static_cast<T*>(p)->~T(); },
          .object = obj,
          .next = m_cleanupHead
      };
        m_cleanupHead = node;
      }

      return obj;
    };

    [[nodiscard]] void *allocateRaw(size_t size, size_t alignment = alignof(std::max_align_t)) {
      return internal_allocate(size, alignment);
    }

    template<typename T = uint8_t>
    [[nodiscard]] std::span<T> allocateSpan(size_t count, size_t alignment = alignof(T)) {
      if (count == 0) [[unlikely]] {
        return {};
      }

      void *ptr = internal_allocate(count * sizeof(T), alignment);

      return std::span<T>(static_cast<T *>(ptr), count);
    }

    void reset() {
      while (m_cleanupHead) {
        m_cleanupHead->destroyer(m_cleanupHead->object);
        m_cleanupHead = m_cleanupHead->next;
      }
      m_pool.release();
    }

  private:
    void* internal_allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
      m_usedBytes += size;
      return m_pool.allocate(size, alignment);
    }
    CleanupNode* m_cleanupHead = nullptr;
    const size_t m_capacity;
    size_t m_usedBytes = 0;
    std::unique_ptr<uint8_t[]> m_backingBuffer;
    std::pmr::monotonic_buffer_resource m_pool;
  };

} // namespace SAP
