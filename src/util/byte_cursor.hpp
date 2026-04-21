//
// Created by oleub on 01.01.26.
//

#pragma once
#include <cstdint>
#include <cstring>
#include <deque>
#include <span>
#include <stdexcept>


namespace SC {
  class ChaosBumpArena;
}

namespace SAA {
  class AssetFileParser;
  struct FName;

  class ByteCursor {
  private:
    [[noreturn]] void throw_overflow(size_t pos, size_t size) const;

  public:
    ByteCursor() = default;

    explicit ByteCursor(std::span<const uint8_t> buf, AssetFileParser *owner = nullptr) : m_owner(owner),
                                                                                          m_buffer(buf) {
    }

    [[nodiscard]] size_t size() const { return m_buffer.size(); }
    [[nodiscard]] size_t absOffset() const { return m_absoluteOffset; }

    void setAbsoluteOffset(size_t offset) { m_absoluteOffset = offset; }

    [[nodiscard]] std::span<const uint8_t> buffer() const { return m_buffer; }
    AssetFileParser *m_owner = nullptr;

    [[nodiscard]] size_t tell() const { return m_cursor; }
    [[nodiscard]] size_t remaining() const { return m_buffer.size() - m_cursor; }

    void skip(int64_t bytes) { seek(m_cursor + bytes); }

    void seek(size_t newPos);

    void align(size_t alignment) {
      size_t mis = m_cursor % alignment;
      if (mis)
        skip(alignment - mis);
    }

    FName readFName();

    bool testReadFName();

    std::span<std::string_view> loadNameBatch();

    std::span<std::string_view> loadNameBatch(SC::ChaosBumpArena *utf16Storage);

    template<typename T>
    std::span<const T> readArrayView();

    [[nodiscard]] ByteCursor subspan(size_t offset, size_t size) const {
      if (offset + size > m_buffer.size())[[unlikely]]{
        throw_overflow(offset, size);
      }

      ByteCursor sub(m_buffer.subspan(offset, size), m_owner);
      sub.setAbsoluteOffset(m_absoluteOffset + offset);
      return sub;
    }

    std::string_view readFString();

    std::string_view readFString(SC::ChaosBumpArena *utf16Storage);

    template<typename T>
    T read() {
      static_assert(std::is_trivial_v<T>, "T must be trivial");

      if (m_cursor + sizeof(T) > m_buffer.size())[[unlikely]]{
      }

      T val;
      std::memcpy(&val, m_buffer.data() + m_cursor, sizeof(T));
      m_cursor += sizeof(T);
      return val;
    }

    bool readBool() { return static_cast<bool>(read<uint32_t>()); }
    bool readFlag() { return static_cast<bool>(read<uint8_t>()); }

    std::string_view readStrView() {
      auto len = read<uint8_t>();
      std::string_view str = {reinterpret_cast<const char *>(m_buffer.data() + m_cursor), len};
      m_cursor += len;
      return str;
    }

    template<typename T>
    std::span<const T> readBulkArray() {
      static_assert(std::is_trivial_v<T>, "T must be trivial");
      auto elementSize = read<uint32_t>();
      auto elementCount = read<uint32_t>();
      auto temp = m_cursor;
      auto array = readArray<T>(elementCount);
      if (m_cursor != temp + elementCount * elementSize) [[unlikely]] {
        throw std::runtime_error("ByteCursor raw array size mismatch");
      }
      return array;
    }

    template<typename T>
    std::span<const T> readArray() {
      static_assert(std::is_trivial_v<T>, "T must be trivial");
      auto count = read<int32_t>();
      return readArray<T>(count);
    }

    template<typename T>
    std::span<const T> readArray(size_t count) {
      static_assert(std::is_trivial_v<T>, "T must be trivial");
      size_t bytes = sizeof(T) * count;

      if (m_cursor + bytes > m_buffer.size()) [[unlikely]] {
        throw_overflow(m_cursor, bytes);
      }

      std::span<const T> out(reinterpret_cast<const T *>(m_buffer.data() + m_cursor), count);

      m_cursor += bytes;
      return out;
    }

    template<typename T>
      requires std::is_integral_v<T>
    ByteCursor &operator+=(T bytes) {
      skip(bytes);
      return *this;
    }

    template<typename T>
      requires std::is_integral_v<T>
    ByteCursor &operator=(T bytes) {
      seek(bytes);
      return *this;
    }

    template<typename T>
      requires std::is_integral_v<T>
    friend ByteCursor operator+(ByteCursor c, T bytes) {
      c += bytes;
      return c;
    }

    template<typename T>
      requires std::is_integral_v<T>
    ByteCursor &operator-=(T bytes) {
      skip(-bytes);
      return *this;
    }

    template<typename T>
      requires std::is_integral_v<T>
    friend ByteCursor operator-(ByteCursor c, T bytes) {
      c -= bytes;
      return c;
    }

  private:
    std::span<const uint8_t> m_buffer;
    uint64_t m_cursor = 0;
    uint64_t m_absoluteOffset = 0;
  };


  template<typename T>
  std::span<const T> ByteCursor::readArrayView() {
    auto initialPos = m_cursor;
    auto count = read<uint32_t>();
    m_cursor = read<uint32_t>() + initialPos;

    auto out = readArray<T>(count);

    m_cursor = initialPos + sizeof(uint32_t) * 2;
    return out;
  }
}
