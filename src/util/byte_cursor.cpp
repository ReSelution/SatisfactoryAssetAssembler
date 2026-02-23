//
// Created by oleub on 01.01.26.
//

#include "byte_cursor.hpp"

#include <deque>

#include "AssetFileParser.hpp"
#include "heap_storage.hpp"

#include "core/name.hpp"

inline std::string utf16ToUtf8(const std::span<const char16_t> &input) {
  if (input.empty())
    return "";

  std::string out;
  out.reserve(input.size()); // Optimistische Annahme: Meist 1:1 Größe bei ASCII

  for (unsigned short cp: input) {
    if (cp < 0x80) {
      out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
      out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
      // Für alles über 0x800 (Standard-Sonderzeichen)
      out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
  }
  return out;
}

namespace SAP {
  void ByteCursor::seek(size_t newPos) {
    if (newPos > m_buffer.size() && newPos > 0) [[unlikely]] {
      throw std::runtime_error("ByteCursor seek out of range");
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

  std::span<std::string_view> ByteCursor::loadNameBatch(HeapStorage *utf16Storage) {
    auto count = read<uint32_t>();
    if (count == 0)
      return {};

    m_cursor += sizeof(uint32_t);         // numStringBytes
    m_cursor += sizeof(uint64_t);         // hashVersion
    m_cursor += count * sizeof(uint64_t); // hashes

    HeapStorage *storage = utf16Storage;
    if (storage == nullptr) {
      storage = m_owner->storage();
    }
    assert(storage != nullptr);

    auto headers = readArray<FSerializedNameHeader>(count);

    auto entries = storage->allocateSpan<std::string_view>(count);

    for (uint32_t i = 0; i < count; ++i) {
      const auto &header = headers[i];
      const uint32_t length = header.Length();

      if (header.IsUtf16()) {
        auto chars = readArray<char16_t>(length);
        auto res = storage->utf16ToUtf8Arena(chars);
        entries[i] = res;
      } else {
        const char *startPos = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
        entries[i] = {startPos, length};
        m_cursor += length;
      }
    }
    return entries;
  }

  std::string_view ByteCursor::readFString(HeapStorage *utf16Storage) {
    auto length = read<int32_t>();
    if (length == 0)
      return {};

    const bool isUnicode = length < 0;
    const auto absLength = static_cast<uint32_t>(isUnicode ? -length : length);
    const uint32_t byteSize = absLength * (isUnicode ? 2u : 1u);

    if (remaining() < byteSize)
      throw std::runtime_error("readFString out of range");

    std::string_view result;

    if (isUnicode) {
      const char16_t *utf16Source = reinterpret_cast<const char16_t *>(m_buffer.data() + m_cursor);
      std::span<const char16_t> src(utf16Source, absLength - 1);
      if (utf16Storage) {
        result = utf16Storage->utf16ToUtf8Arena(src);
      } else if (m_owner) {
        result = m_owner->storage()->utf16ToUtf8Arena(src);
      } else {
        throw std::runtime_error("readFString No Storage");
      }
    } else {
      if (absLength > 1) {
        result = std::string_view(reinterpret_cast<const char *>(m_buffer.data() + m_cursor), absLength - 1);
      }
    }

    m_cursor += byteSize;
    return result;
  }
} // namespace SAP
