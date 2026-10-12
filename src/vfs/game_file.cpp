//
// Created by oleub on 17.04.26.
//

#include "game_file.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <thread>
#include <vector>

#include "sc/threading/threading.hpp"

#include "core/name.hpp"
#include "runtime_options.hpp"

namespace SAA {
  GameFile::GameFile(sc::MonotonicResource *storage) : m_storage(storage) {}

  ByteCursor GameFile::subspan(size_t offset, size_t size) {
    if (!loadRange(offset, size)) [[unlikely]] {
      if (offset + size > impl->fileSize) {
        throw std::runtime_error("subspan out of range");
      }
      // In range, so a chunk failed: the read, or the decompression of a damaged block.
      throw std::runtime_error("chunk of the file could not be read or decompressed");
    }

    ByteCursor sub(m_buffer.subspan(offset, size), m_owner);
    sub.setAbsoluteOffset(offset);
    return sub;
  }

  void GameFile::readInto(size_t offset, std::span<uint8_t> dst) {
    if (!impl->readInto(offset, dst, m_buffer)) [[unlikely]] {
      if (offset + dst.size() > impl->fileSize) {
        throw std::runtime_error("readInto out of range");
      }
      throw std::runtime_error("chunk of the file could not be read or decompressed");
    }
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
      const auto &header = headers[i];
      const uint32_t length = header.Length();

      if (header.IsUtf16()) {
        auto chars = readArray<char16_t>(length);
        auto res = utf16ToUtf8(*utf16Storage, chars);
        entries[i] = res;
      } else {
        if (!load(length)) [[unlikely]] {
          throw std::runtime_error("Name batch entry out of range");
        }
        const char *startPos = reinterpret_cast<const char *>(m_buffer.data() + m_cursor);
        entries[i] = {startPos, length};
        m_cursor += length;
      }
    }
    return entries;
  }


  GameFileImpl::GameFileImpl(const size_t fileSize) : fileSize(fileSize) {}

  bool GameFileImpl::isLoaded(size_t targetOffset, size_t size, size_t &index) {
    const size_t targetEnd = targetOffset + size;

    // The chunk targetOffset lies in: the one of the last request or the one behind it when a file is read in
    // order, anything else is searched for.
    if (index >= chunkOffsetsLoaded.size() || targetOffset < chunkStart(index) || targetOffset >= chunkEnd(index)) {
      if (index + 1 < chunkOffsetsLoaded.size() && targetOffset >= chunkStart(index + 1) &&
          targetOffset < chunkEnd(index + 1)) {
        ++index;
      } else {
        size_t found = 0;
        for (size_t low = 0, high = chunkOffsetsLoaded.size(); low < high;) {
          const size_t mid = low + ((high - low) / 2);
          if (chunkStart(mid) <= targetOffset) {
            found = mid;
            low = mid + 1;
          } else {
            high = mid;
          }
        }
        index = found;
      }
    }

    for (size_t i = index; i < chunkOffsetsLoaded.size(); ++i) {
      if (!(chunkOffsetsLoaded[i].load(std::memory_order_acquire) & FINISHED_BIT)) {
        return false;
      }
      const size_t loadedTo = chunkEnd(i);
      if (loadedTo >= targetEnd) {
        // Only a range that starts inside the prefix makes it longer.
        size_t prefix = loadedPrefix.load(std::memory_order_acquire);
        while (targetOffset <= prefix && loadedTo > prefix &&
               !loadedPrefix.compare_exchange_weak(prefix, loadedTo, std::memory_order_acq_rel)) {
        }
        return true;
      }
    }

    return false;
  }

  bool GameFileImpl::safeLoad(size_t targetOffset, size_t size, std::span<uint8_t> dst) {
    // The chunk walks below do not stop at the last chunk on their own.
    if (targetOffset + size > fileSize) [[unlikely]] {
      return false;
    }
    auto index = idx.load(std::memory_order_relaxed);
    if (isLoaded(targetOffset, size, index)) {
      idx.store(index, std::memory_order_relaxed);
      return true;
    }
    return tryLoad(targetOffset, size, index, dst) && waitFinished(targetOffset, size, index);
  }

  namespace {
    // Below this many chunks the calling thread loads them alone, a helper would hardly get started in time.
    constexpr size_t MIN_CHUNKS_FOR_HELPERS = 4;

    // Calls run(0) ... run(count - 1), on this thread and on pool threads that have nothing else to do, and returns
    // once all of them are through. See sc::threading::parallelFor(): this may be called from a pool task, and a
    // run() that throws keeps the chunks nobody has started from being loaded.
    // Measured on the test parse of 1017 (28214 files, 15 threads): 3.6 s -> 2.0 s against loading the chunks one
    // after the other, the CPU time stays the same. What is left of the tail runs on 3 to 5 threads: the payloads
    // of a big file are asked for one after the other, and each of them only spans a few chunks.
    template<typename F>
    void forEachChunk(size_t count, F &&run) {
      const size_t helpers = count >= MIN_CHUNKS_FOR_HELPERS && !runtimeOptions().serial ? SIZE_MAX : 0;
      sc::threading::parallelFor(count, 1, helpers, [&](size_t first, size_t last) {
        for (size_t n = first; n < last; ++n) {
          run(n);
        }
      });
    }
  } // namespace

  bool GameFileImpl::tryLoad(size_t targetOffset, size_t size, size_t chunIdx, std::span<uint8_t> dst) {
    const size_t targetEnd = targetOffset + size;

    // Claims every chunk of the range that nobody has loaded or is loading yet. The others are finished, failed or
    // in the hands of another thread, waitFinished() waits for those.
    std::vector<size_t> chunksToLoad;
    bool failed = false;
    for (size_t cIdx = chunIdx; cIdx < chunkOffsetsLoaded.size() && chunkStart(cIdx) < targetEnd; ++cIdx) {
      auto &atomic = chunkOffsetsLoaded[cIdx];
      auto expected = atomic.load(std::memory_order_relaxed);
      if (expected & FAILED_BIT) {
        failed = true;
        continue;
      }
      if (expected & (FINISHED_BIT | LOADING_BIT)) {
        continue;
      }
      if (atomic.compare_exchange_strong(expected, expected | LOADING_BIT, std::memory_order_acq_rel)) {
        chunksToLoad.emplace_back(cIdx);
      } else if (expected & FAILED_BIT) {
        failed = true;
      }
    }

    if (chunksToLoad.size() == 1) {
      willNeed(chunksToLoad.front());
    }
    // Every claimed chunk ends up FINISHED or FAILED, otherwise the threads in waitFinished() never get out. So
    // an exception is kept until all of them are through instead of ending the loop.
    std::atomic<bool> loadFailed{false};
    std::atomic_flag threw;
    std::exception_ptr error;
    forEachChunk(chunksToLoad.size(), [&](size_t n) {
      const size_t i = chunksToLoad[n];
      bool loaded = false;
      try {
        const size_t begin = chunkStart(i);
        loaded = loadChunk(i, dst.subspan(begin, chunkEnd(i) - begin));
      } catch (...) {
        if (!threw.test_and_set(std::memory_order_acq_rel)) {
          error = std::current_exception();
        }
      }
      chunkOffsetsLoaded[i].fetch_or(loaded ? FINISHED_BIT : FAILED_BIT, std::memory_order_release);
      if (!loaded) [[unlikely]] {
        loadFailed.store(true, std::memory_order_relaxed);
      }
    });
    if (error) {
      std::rethrow_exception(error);
    }

    // What another thread is loading is reported by waitFinished().
    return !failed && !loadFailed.load(std::memory_order_relaxed);
  }

  bool GameFileImpl::readInto(size_t offset, std::span<uint8_t> dst, std::span<const uint8_t> fileBuffer) {
    const size_t rangeEnd = offset + dst.size();
    if (rangeEnd > fileSize || rangeEnd < offset) [[unlikely]] {
      return false;
    }
    if (dst.empty()) {
      return true;
    }

    // The last chunk that starts at or before offset.
    size_t first = 0;
    for (size_t low = 0, high = chunkOffsetsLoaded.size(); low < high;) {
      const size_t mid = low + (high - low) / 2;
      if (chunkStart(mid) <= offset) {
        first = mid;
        low = mid + 1;
      } else {
        high = mid;
      }
    }
    size_t count = 0;
    while (first + count < chunkOffsetsLoaded.size() && chunkStart(first + count) < rangeEnd) {
      ++count;
    }

    std::atomic<bool> loadFailed{false};
    forEachChunk(count, [&](size_t n) {
      const size_t i = first + n;
      const size_t begin = chunkStart(i);
      const size_t end = chunkEnd(i);
      // The part of the chunk that lies in the range, and where it goes in dst.
      const size_t from = std::max(begin, offset);
      const size_t to = std::min(end, rangeEnd);
      uint8_t *target = dst.data() + (from - offset);

      const uint64_t state = chunkOffsetsLoaded[i].load(std::memory_order_acquire);
      if (state & FINISHED_BIT) {
        std::memcpy(target, fileBuffer.data() + from, to - from);
        return;
      }
      if (from == begin && to == end) {
        if (!loadChunk(i, {target, end - begin})) [[unlikely]] {
          loadFailed.store(true, std::memory_order_relaxed);
        }
        return;
      }
      // A chunk at the edge: the decompressor only gives it out whole, with the bytes in front of or behind the
      // range. At most two per call.
      thread_local std::vector<uint8_t> edge;
      if (edge.size() < end - begin) {
        edge.resize(end - begin);
      }
      if (!loadChunk(i, {edge.data(), end - begin})) [[unlikely]] {
        loadFailed.store(true, std::memory_order_relaxed);
        return;
      }
      std::memcpy(target, edge.data() + (from - begin), to - from);
    });
    return !loadFailed.load(std::memory_order_relaxed);
  }

  bool GameFileImpl::waitFinished(size_t targetOffset, size_t size, size_t chunIdx) {
    for (size_t offset = targetOffset; offset < targetOffset + size;) {
      auto &atomic = chunkOffsetsLoaded[chunIdx];
      size_t nextChunkStart = (chunIdx + 1 < chunkOffsetsLoaded.size())
                                  ? (chunkOffsetsLoaded[chunIdx + 1].load(std::memory_order_relaxed) & VALUE_MASK)
                                  : fileSize;
      offset = nextChunkStart;
      // TODO: Vorher hat der wartende Thread ueber SC::ChaosThreading::helpThread andere Tasks abgearbeitet.
      // sc::threading bietet das nicht mehr oeffentlich an (helpThread liegt in impl), deshalb wird hier nur noch
      // die CPU abgegeben.
      uint64_t state = atomic.load(std::memory_order_acquire);
      while (!(state & (FINISHED_BIT | FAILED_BIT))) {
        std::this_thread::yield();
        state = atomic.load(std::memory_order_acquire);
      }
      if (state & FAILED_BIT) [[unlikely]] {
        return false;
      }
      chunIdx++;
    }
    return true;
  }
} // namespace SAA
