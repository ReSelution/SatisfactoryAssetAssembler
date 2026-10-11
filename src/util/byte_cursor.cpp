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
        const char *startPos = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
        entries[i]           = {startPos, length};
        m_cursor += length;
      }
    }
    return entries;
  }

  std::string_view ByteCursor::readFString() {
    assert(nullptr != m_owner);
    return readFString(m_owner->storage());
  }

  // TODO: Beide readFString-Varianten mit Laenge pruefen nicht, ob length noch im Puffer liegt. Bei kaputten oder
  // falsch interpretierten Daten zeigt die zurueckgegebene View hinter den Puffer (Out-of-bounds-Read) und m_cursor
  // steht ausserhalb. readArray<>() und read<>() pruefen das, hier fehlt es. War vor der Portierung schon so.
  std::string_view ByteCursor::readFString(sc::MonotonicResource *utf16Storage) {
    auto length = read<int32_t>();
    if (length == 0) [[unlikely]] {
      return {};
    }

    if (length < 0) {
      const auto absLength    = static_cast<uint32_t>(-length);
      const uint32_t byteSize = absLength * 2u;
      const auto *utf16Source = reinterpret_cast<const char16_t *>(m_buffer.data() + m_cursor);
      m_cursor += byteSize;
      return utf16ToUtf8(*utf16Storage, {utf16Source, absLength - 1});
    }

    const char *asciiSource = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
    m_cursor += length;
    return {asciiSource, static_cast<size_t>(length - 1)};
  }

  std::string_view ByteCursor::readFString(sc::Magpie *magpie) {
    auto length = read<int32_t>();
    if (length == 0) [[unlikely]] {
      return {};
    }

    if (length < 0) {
      const auto absLength    = static_cast<uint32_t>(-length);
      const uint32_t byteSize = absLength * 2u;
      const auto *utf16Source = reinterpret_cast<const char16_t *>(m_buffer.data() + m_cursor);
      m_cursor += byteSize;
      return magpie->storeStrUTF16(utf16Source, absLength - 1);
    }

    const char *asciiSource = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
    m_cursor += length;
    return magpie->storeStr({asciiSource, static_cast<size_t>(length - 1)});
  }
} // namespace SAA
