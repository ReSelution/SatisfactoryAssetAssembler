//
// Created by oleub on 17.04.26.
//

#include "game_file.hpp"
#include "core/name.hpp"

namespace SAA {
  GameFile::GameFile(SC::ChaosBumpArena *storage):m_storage(storage) {

  }

  bool GameFile::load(size_t size) {
    if (m_cursor + size <= impl->end) {
      return true;
    }

    return impl->load(m_cursor, size, m_buffer);
  }

  void GameFile::seek(size_t newPos) {
    if (newPos > m_buffer.size()) [[unlikely]] {
      throw std::runtime_error("ByteCursor seek out of range");
    }
    m_cursor = newPos;
  }

  std::span<std::string_view> GameFile::loadNameBatch() {
    return loadNameBatch(m_storage);
  }


  std::span<std::string_view> GameFile::loadNameBatch(SC::ChaosBumpArena *utf16Storage) {
    auto count = read<uint32_t>();
    if (count == 0)
      return {};

    seek(m_cursor + sizeof(uint32_t) + sizeof(uint64_t) + count * sizeof(uint64_t));

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
        load(length);
        const char *startPos = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
        entries[i] = {startPos, length};
        m_cursor += length;
      }
    }
    return entries;
  }




  GameFileImpl::GameFileImpl(const size_t fileSize)
          : fileSize(fileSize) {


  }


} // SAA