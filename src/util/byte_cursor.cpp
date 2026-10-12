//
// Created by oleub on 01.01.26.
//

#include "byte_cursor.hpp"

#include <cassert>
#include <format>
#include <string_view>

#include <simdutf.h>

#include "AssetFileParser.hpp"
#include "core/name.hpp"
#include "sc/localizations/magpie_translator.hpp"
#include "sc/memory/monotonic_resource.hpp"

namespace SAA {
  std::string_view utf16ToUtf8(sc::MonotonicResource &storage, std::span<const char16_t> chars) {
    if (chars.empty()) {
      return {};
    }
    const size_t requiredSize = simdutf::utf8_length_from_utf16(chars.data(), chars.size());
    auto target               = storage.allocateSpan<char>(requiredSize);
    // Gibt bei ungueltigem UTF-16 0 zurueck, der Name ist dann leer.
    const size_t written = simdutf::convert_utf16_to_utf8(chars.data(), chars.size(), target.data());
    return {target.data(), written};
  }

  void ByteCursor::throw_overflow(size_t pos, size_t requested) const {
    throw std::out_of_range(
        std::format("Buffer Overflow: Requested {} bytes @ {}/{}", requested, pos, m_buffer.size()));
  }

  void ByteCursor::seek(size_t newPos) {
    if (newPos > m_buffer.size()) [[unlikely]] {
      throw_overflow(m_cursor, newPos - m_cursor);
    }
    m_cursor = newPos;
  }


  bool ByteCursor::testReadFName() {
    auto pos = tell();
    FName name{
        .index  = read<uint32_t>(),
        .number = read<uint32_t>(),
    };
    m_cursor = pos;
    return name.index < m_owner->nameMap().size() && name.number < 256;
  }

  std::span<std::string_view> ByteCursor::loadNameBatch() {
    assert(nullptr != m_owner);
    return loadNameBatch(m_owner->storage());
  }


  std::span<std::string_view> ByteCursor::loadNameBatch(sc::MonotonicResource *utf16Storage) {
    auto count = read<uint32_t>();
    if (count == 0) [[unlikely]] {
      return {};
    }

    skip(static_cast<int64_t>(sizeof(uint64_t) + sizeof(uint32_t) + count * sizeof(uint64_t)));

    assert(utf16Storage != nullptr);

    auto headers = readArray<FSerializedNameHeader>(count);

    auto entries = utf16Storage->allocateSpan<std::string_view>(count);

    for (uint32_t i = 0; i < count; ++i) {
      const auto &header    = headers[i];
      const uint32_t length = header.Length();

      if (header.IsUtf16()) {
        auto chars = readArray<char16_t>(length);
        auto res   = utf16ToUtf8(*utf16Storage, chars);
        entries[i] = res;
      } else {
        auto chars = readArray<char>(length);
        entries[i] = {chars.data(), chars.size()};
      }
    }
    return entries;
  }

  std::string_view ByteCursor::readFString() {
    assert(nullptr != m_owner);
    return readFString(m_owner->storage());
  }

  // The length of an FString counts the terminator. Negative means UTF-16 and counts char16_t.
  std::string_view ByteCursor::readFString(sc::MonotonicResource *utf16Storage) {
    auto length = read<int32_t>();
    if (length == 0) [[unlikely]] {
      return {};
    }

    if (length < 0) {
      // Widened before negating: -INT32_MIN does not fit into an int32_t.
      const auto chars = readArray<char16_t>(static_cast<size_t>(-static_cast<int64_t>(length)));
      return utf16ToUtf8(*utf16Storage, chars.first(chars.size() - 1));
    }

    const auto chars = readArray<char>(static_cast<size_t>(length));
    return {chars.data(), chars.size() - 1};
  }

  std::string_view ByteCursor::readFString(sc::Magpie *magpie) {
    auto length = read<int32_t>();
    if (length == 0) [[unlikely]] {
      return {};
    }

    if (length < 0) {
      const auto chars = readArray<char16_t>(static_cast<size_t>(-static_cast<int64_t>(length)));
      return magpie->storeStrUTF16(chars.data(), chars.size() - 1);
    }

    const auto chars = readArray<char>(static_cast<size_t>(length));
    return magpie->storeStr({chars.data(), chars.size() - 1});
  }
} // namespace SAA
