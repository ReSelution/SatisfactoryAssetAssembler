//
// Created by oleub on 17.04.26.
//

#include "game_file.hpp"

#include <cassert>
#include <thread>
#include <vector>

#include "core/name.hpp"

namespace SAA {
  GameFile::GameFile(sc::MonotonicResource *storage) : m_storage(storage) {}

  ByteCursor GameFile::subspan(size_t offset, size_t size) {
    auto old = m_cursor;
    m_cursor = offset;
    if (!load(size)) [[unlikely]] {
      throw std::runtime_error("subspan out of range");
    }

    ByteCursor sub(m_buffer.subspan(offset, size), m_owner);
    sub.setAbsoluteOffset(offset);
    m_cursor = old;
    return sub;
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

  std::span<std::string_view> GameFile::loadNameBatch() { return loadNameBatch(m_storage); }


  std::span<std::string_view> GameFile::loadNameBatch(sc::MonotonicResource *utf16Storage) {
    auto count = read<uint32_t>();
    if (count == 0)
      return {};

    seek(m_cursor + sizeof(uint32_t) + sizeof(uint64_t) + count * sizeof(uint64_t));

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
        // TODO: Der Rueckgabewert von load() wird ignoriert. Schlaegt das Laden fehl oder liegt length hinter dem
        // Dateiende, zeigt die View unten auf nicht geladene bzw. fremde Bytes. War vor der Portierung schon so.
        load(length);
        const char *startPos = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
        entries[i]           = {startPos, length};
        m_cursor += length;
      }
    }
    return entries;
  }


  GameFileImpl::GameFileImpl(const size_t fileSize) : fileSize(fileSize) {}

  bool GameFileImpl::isLoaded(size_t targetOffset, size_t size, size_t &index) {
    const size_t targetEnd = targetOffset + size;
    bool foundStart        = false;

    if (index == chunkOffsetsLoaded.size()) {
      index = 0;
    }
    const size_t currentStart = chunkOffsetsLoaded[index].load(std::memory_order_relaxed) & VALUE_MASK;
    if (targetOffset < currentStart) [[unlikely]] {
      index = 0;
    }

    for (size_t i = index; i < chunkOffsetsLoaded.size(); ++i) {
      const auto &entry              = chunkOffsetsLoaded[i];
      const size_t currentChunkStart = entry.load(std::memory_order_relaxed) & VALUE_MASK;
      if (!foundStart) {
        size_t nextChunkStart = (i + 1 < chunkOffsetsLoaded.size())
                                    ? (chunkOffsetsLoaded[i + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                                    : fileSize;

        if (targetOffset >= currentChunkStart && targetOffset < nextChunkStart) {
          index      = i;
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
    size_t cIdx = chunIdx;

    size_t nextChunkStart = (cIdx + 1 < chunkOffsetsLoaded.size())
                                ? (chunkOffsetsLoaded[cIdx + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                                : fileSize;
    if (nextChunkStart < targetOffset + size) {

      std::vector<size_t> chunksToLoad;
      chunksToLoad.reserve(chunkOffsetsLoaded.size());
      for (size_t offset = targetOffset; offset < targetOffset + size;) {
        auto &atomic          = chunkOffsetsLoaded[cIdx];
        auto expected         = atomic.load(std::memory_order_relaxed);
        size_t nextChunkStart = (cIdx + 1 < chunkOffsetsLoaded.size())
                                    ? (chunkOffsetsLoaded[cIdx + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                                    : fileSize;
        offset                = nextChunkStart;
        if (expected & FINISHED_BIT) {
          cIdx++;
          continue;
        };
        if (!(expected & LOADING_BIT)) {
          uint64_t desired = expected | LOADING_BIT;
          if (atomic.compare_exchange_strong(expected, desired, std::memory_order_acq_rel)) {
            chunksToLoad.emplace_back(cIdx);
          }
          cIdx++;
        }
      }
      // TODO: Die Chunks werden hier nacheinander im aufrufenden Thread geladen. Vorher lief das ueber
      // SC::ChaosThreading::parralelFor. Sobald sc::threading::parallelFor(count, grain, fn(first, last)) aus dem
      // ecs-Branch von StructuredChaos auf master ist, hier wieder parallel laden: es blockiert, darf aus Pool-Tasks
      // aufgerufen werden und der Aufrufer arbeitet selbst mit, die Worker koennen sich also nicht gegenseitig
      // blockieren.
      for (size_t i: chunksToLoad) {
        if (load(i, dst)) [[likely]] {
          chunkOffsetsLoaded[i].fetch_or(FINISHED_BIT, std::memory_order_release);
        }
      }
    } else {
      auto &atomic  = chunkOffsetsLoaded[cIdx];
      auto expected = atomic.load(std::memory_order_relaxed);
      if (!(expected & LOADING_BIT)) {
        uint64_t desired = expected | LOADING_BIT;
        if (atomic.compare_exchange_strong(expected, desired, std::memory_order_acq_rel)) {
          willNeed(cIdx);
          if (load(cIdx, dst)) [[likely]] {
            atomic.fetch_or(FINISHED_BIT, std::memory_order_release);
            return true;
          }
        }
      }
    }

    // TODO: Gibt auch dann true zurueck, wenn load() fehlgeschlagen ist. FINISHED_BIT wird dann nie gesetzt und
    // waitFinished() dreht sich fuer diesen und alle anderen wartenden Threads endlos. War vor der Portierung schon so.
    return true;
  }

  void GameFileImpl::waitFinished(size_t targetOffset, size_t size, size_t chunIdx) {
    for (size_t offset = targetOffset; offset < targetOffset + size;) {
      auto &atomic          = chunkOffsetsLoaded[chunIdx];
      size_t nextChunkStart = (chunIdx + 1 < chunkOffsetsLoaded.size())
                                  ? (chunkOffsetsLoaded[chunIdx + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                                  : fileSize;
      offset                = nextChunkStart;
      // TODO: Vorher hat der wartende Thread ueber SC::ChaosThreading::helpThread andere Tasks abgearbeitet.
      // sc::threading bietet das nicht mehr oeffentlich an (helpThread liegt in impl), deshalb wird hier nur noch
      // die CPU abgegeben.
      while (!(atomic.load(std::memory_order_acquire) & FINISHED_BIT)) {
        std::this_thread::yield();
      }
      chunIdx++;
    }
  }
} // namespace SAA
