//
// Created by oleub on 01.01.26.
//

#include "byte_cursor.hpp"

#include <cassert>

#include "AssetFileParser.hpp"
#include "memory/chaos_bump_arena.hpp"
#include "core/name.hpp"

namespace SAA {
  void ByteCursor::throw_overflow(size_t pos, size_t requested) const {
    throw std::out_of_range(std::format(
      "Buffer Overflow: Requested {} bytes @ {}/{}",
      requested, pos, m_buffer.size()
    ));
  }

  void ByteCursor::seek(size_t newPos) {
    if (newPos > m_buffer.size()) [[unlikely]] {
      throw_overflow(m_cursor, 0);
    }
    m_cursor = newPos;
  }

  FName ByteCursor::readFName() {
    FName name{
      .index = read<uint32_t>(),
      .number = read<uint32_t>(),
    };
    name.plainTextView = m_owner->nameFromMap(name.index);
    return name;
  }

  bool ByteCursor::testReadFName() {
    auto pos = tell();
    FName name{
      .index = read<uint32_t>(),
      .number = read<uint32_t>(),
    };
    m_cursor = pos;
    return name.index < m_owner->nameMap().size() && name.number < 256;
  }

  std::span<std::string_view> ByteCursor::loadNameBatch() {
    assert(nullptr != m_owner);
    return loadNameBatch(m_owner->storage());
  }


  std::span<std::string_view> ByteCursor::loadNameBatch(SC::ChaosBumpArena *utf16Storage) {
    auto count = read<uint32_t>();
    if (count == 0) [[unlikely]] { return {}; }

    skip(static_cast<int64_t>(sizeof(uint64_t) + sizeof(uint32_t) + count * sizeof(uint64_t)));

    assert(utf16Storage != nullptr);

    auto headers = readArray<FSerializedNameHeader>(count);

    auto entries = utf16Storage->allocateSpan<std::string_view>(count);

    for (uint32_t i = 0; i < count; ++i) {
      const auto &header = headers[i];
      const uint32_t length = header.Length();

      if (header.IsUtf16()) {
        auto chars = readArray<char16_t>(length);
        auto res = utf16Storage->utf16ToUtf8(chars);
        entries[i] = res;
      } else {
        const char *startPos = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
        entries[i] = {startPos, length};
        m_cursor += length;
      }
    }
    return entries;
  }

  std::string_view ByteCursor::readFString() {
    assert(nullptr != m_owner);
    return readFString(m_owner->storage());
  }

  std::string_view ByteCursor::readFString(SC::ChaosBumpArena *utf16Storage) {
    auto length = read<int32_t>();
    if (length == 0) [[unlikely]] { return {}; }

    if (length < 0) {
      const auto absLength = static_cast<uint32_t>(-length);
      const uint32_t byteSize = absLength * 2u;
      const auto *utf16Source = reinterpret_cast<const char16_t *>(m_buffer.data() + m_cursor);
      m_cursor += byteSize;
      return utf16Storage->utf16ToUtf8({utf16Source, absLength - 1});
    }

    const char *asciiSource = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
    m_cursor += length;
    return {asciiSource, static_cast<size_t>(length - 1)};
  }
}
