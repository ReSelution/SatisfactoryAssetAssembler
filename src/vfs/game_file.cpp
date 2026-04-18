//
// Created by oleub on 17.04.26.
//

#include <random>
#include "game_file.hpp"
#include "core/name.hpp"

namespace SAA {
  GameFile::GameFile(SC::ChaosBumpArena *storage) : m_storage(storage) {

  }

  bool GameFile::load(size_t size) {
    if (m_cursor + size <= impl->end) {
      return true;
    }

    return impl->safeLoad(m_cursor, size, m_buffer);
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

  bool GameFileImpl::isLoaded(size_t targetOffset, size_t size, size_t &index) {

    const size_t targetEnd = targetOffset + size;
    bool foundStart = false;

    if (index == chunkOffsetsLoaded.size()) {
      index = 0;
    }
    const size_t currentStart = chunkOffsetsLoaded[index].load(std::memory_order_relaxed) & VALUE_MASK;
    if (targetOffset < currentStart) [[unlikely]] {
      index = 0;
    }

    for (size_t i = index; i < chunkOffsetsLoaded.size(); ++i) {
      const auto &entry = chunkOffsetsLoaded[i];
      const size_t currentChunkStart = entry.load(std::memory_order_relaxed) & VALUE_MASK;
      if (!foundStart) {
        size_t nextChunkStart = (i + 1 < chunkOffsetsLoaded.size())
                                ? (chunkOffsetsLoaded[i + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                                : fileSize;

        if (targetOffset >= currentChunkStart && targetOffset < nextChunkStart) {
          index = i;
          foundStart = true;
        } else {
          continue;
        }
      }

      if (!(entry & FINISHED_BIT)) {
        return false;
      }

      size_t nextChunkStart = (i + 1 < chunkOffsetsLoaded.size())
                              ? (chunkOffsetsLoaded[i + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                              : fileSize;

      if (nextChunkStart >= targetEnd) {
        end = nextChunkStart - 1;
        return true;
      }
    }

    return false;
  }

  bool GameFileImpl::safeLoad(size_t targetOffset, size_t size, std::span<uint8_t> dst) {
    auto index = idx.load(std::memory_order_relaxed);
    if (isLoaded(targetOffset, size, index)) {
      idx.store(index, std::memory_order_relaxed);
      return true;
    }
    if (tryLoad(targetOffset, size, index, dst)) {
      waitFinished(targetOffset, size, index);
      return true;
    }
    return false;
  }

  bool GameFileImpl::tryLoad(size_t targetOffset, size_t size, size_t chunIdx, std::span<uint8_t> dst) {
    for (size_t offset = targetOffset; offset < targetOffset + size;) {
      auto &atomic = chunkOffsetsLoaded[chunIdx];
      auto expected = atomic.load(std::memory_order_relaxed);

      size_t nextChunkStart = (chunIdx + 1 < chunkOffsetsLoaded.size())
                              ? (chunkOffsetsLoaded[chunIdx + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                              : fileSize;
      offset = nextChunkStart;
      if (expected & FINISHED_BIT) {
        chunIdx++;
      };
      if (!(expected & LOADING_BIT)) {
        uint64_t desired = expected | LOADING_BIT;
        if (atomic.compare_exchange_strong(expected, desired, std::memory_order_acq_rel)) {
          if (load(chunIdx, dst)) {
            atomic.fetch_or(FINISHED_BIT, std::memory_order_release);
          } else { return false; }
        }
        chunIdx++;
      }

    }

    return true;
  }

  void GameFileImpl::waitFinished(size_t targetOffset, size_t size, size_t chunIdx) {
    for (size_t offset = targetOffset; offset < targetOffset + size;) {
      auto &atomic = chunkOffsetsLoaded[chunIdx];
      size_t nextChunkStart = (chunIdx + 1 < chunkOffsetsLoaded.size())
                              ? (chunkOffsetsLoaded[chunIdx + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                              : fileSize;
      offset = nextChunkStart;
      while (!(atomic.load(std::memory_order_acquire) & FINISHED_BIT)) {
#if defined(__x86_64__)
        _mm_pause();
#endif
      }
      chunIdx++;
    }
  }
} // SAA