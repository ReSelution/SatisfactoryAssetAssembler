//
// Created by oleub on 17.04.26.
//

#pragma once
#include <algorithm>

#include <atomic>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "byte_cursor.hpp"
#include "sc/memory/monotonic_resource.hpp"


namespace SAA {
  class AssetFileParser;


  struct GameFileImpl {
    virtual ~GameFileImpl() = default;

    static constexpr uint64_t LOADING_BIT  = 1ULL << 63;
    static constexpr uint64_t FINISHED_BIT = 1ULL << 62;
    // load() failed for this chunk. It is not retried, every read of it fails.
    static constexpr uint64_t FAILED_BIT   = 1ULL << 61;
    static constexpr uint64_t VALUE_MASK   = ~(LOADING_BIT | FINISHED_BIT | FAILED_BIT);

    const size_t fileSize;
    // Every chunk but the last one stands for this many bytes of the file, so the chunk of an offset is a division.
    // Both containers cut a file into compression blocks of one size.
    const size_t chunkSize;
    // Every byte in front of this offset is loaded. Lets a file that is read from front to back skip the look at
    // the chunks. Says nothing about what lies behind it: a range further back can be loaded as well.
    std::atomic<size_t> loadedPrefix = 0;
    std::span<std::atomic<uint64_t>> chunkOffsetsLoaded{};

    GameFileImpl(size_t fileSize, size_t chunkSize);

    [[nodiscard]] bool isLoaded(size_t targetOffset, size_t size, size_t &index);

    bool tryLoad(size_t targetOffset, size_t size, size_t chunIdx, std::span<uint8_t> dst);
    [[nodiscard]] bool waitFinished(size_t targetOffset, size_t size, size_t chunIdx);
    bool safeLoad(size_t offset, size_t size, std::span<uint8_t> dst);

    // Writes the bytes [offset, offset + dst.size()) of the file to dst. fileBuffer is the buffer of the file, a
    // chunk that is already loaded there is copied instead of read again. Loads nothing into fileBuffer itself.
    // False if the range is outside the file or a chunk could not be read.
    [[nodiscard]] bool readInto(size_t offset, std::span<uint8_t> dst, std::span<const uint8_t> fileBuffer);

    // The chunk an offset of the file lies in.
    [[nodiscard]] size_t chunkAt(size_t offset) const {
      const size_t count = chunkOffsetsLoaded.size();
      if (count == 0 || chunkSize == 0) [[unlikely]] {
        return 0;
      }
      return std::min(offset / chunkSize, count - 1);
    }

    // The bytes of the file a chunk stands for.
    [[nodiscard]] size_t chunkStart(size_t idx) const {
      return chunkOffsetsLoaded[idx].load(std::memory_order_relaxed) & VALUE_MASK;
    }
    [[nodiscard]] size_t chunkEnd(size_t idx) const {
      return idx + 1 < chunkOffsetsLoaded.size() ? chunkStart(idx + 1) : fileSize;
    }

    // Reads the chunk into out, which is exactly as long as the chunk. Called from several threads at the same
    // time, for different chunks.
    virtual bool loadChunk(size_t idx, std::span<uint8_t> out) = 0;
    // Announces the chunk to the OS, loadChunk() for it follows right away.
    virtual void willNeed(size_t idx) = 0;

    virtual uint64_t chunks() = 0;
  };

  class GameFile {
  public:
    GameFile() = default;

    GameFile(sc::MonotonicResource *storage);

    [[nodiscard]] size_t tell() const { return m_cursor; }

    [[nodiscard]] size_t size() const { return impl->fileSize; }

    void skip(int64_t bytes) { seek(m_cursor + bytes); }

    void seek(size_t newPos);


    template<typename T>
    T read() {
      static_assert(std::is_trivial_v<T>, "T must be trivial");

      if (!load(sizeof(T))) [[unlikely]] {
        throw std::runtime_error("Buffer Overflow @ " + std::to_string(m_cursor) + "/" +
                                 std::to_string(m_buffer.size()));
      }

      T val;
      std::memcpy(&val, m_buffer.data() + m_cursor, sizeof(T));
      m_cursor += sizeof(T);
      return val;
    }

    template<typename T>
    std::span<const T> readArrayView() {
      auto initialPos = m_cursor;
      auto count      = read<uint32_t>();
      m_cursor        = read<uint32_t>() + initialPos;

      auto out = readArray<T>(count);

      m_cursor = initialPos + sizeof(uint32_t) * 2;
      return out;
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

      if (!load(bytes)) [[unlikely]] {
        throw std::runtime_error("ByteCursor array out of range");
      }

      std::span<const T> out(reinterpret_cast<const T *>(m_buffer.data() + m_cursor), count);

      m_cursor += bytes;
      return out;
    }

    std::span<std::string_view> loadNameBatch();

    std::span<std::string_view> loadNameBatch(sc::MonotonicResource *utf16Storage);


    template<typename T>
      requires std::is_integral_v<T>
    GameFile &operator+=(T bytes) {
      skip(bytes);
      return *this;
    }

    template<typename T>
      requires std::is_integral_v<T>
    GameFile &operator=(T bytes) {
      seek(bytes);
      return *this;
    }

    template<typename T>
      requires std::is_integral_v<T>
    friend GameFile operator+(GameFile c, T bytes) {
      c += bytes;
      return c;
    }

    template<typename T>
      requires std::is_integral_v<T>
    GameFile &operator-=(T bytes) {
      skip(-bytes);
      return *this;
    }

    template<typename T>
      requires std::is_integral_v<T>
    friend GameFile operator-(GameFile c, T bytes) {
      c -= bytes;
      return c;
    }

    template<typename T, typename... Args>
      requires std::derived_from<T, GameFileImpl>
    T *createImpl(Args &&...args) {
      impl                     = m_storage->make<T>(std::forward<Args>(args)...);
      impl->chunkOffsetsLoaded = m_storage->allocateSpan<std::atomic<uint64_t>>(impl->chunks());
      m_buffer                 = m_storage->allocateSpan(impl->fileSize);
      return static_cast<T *>(impl);
    }

    [[nodiscard]] ByteCursor subspan(size_t offset, size_t size);

    // Writes the bytes [offset, offset + dst.size()) of the file straight to dst, without keeping them in the
    // storage of the file. For a target the caller owns, a mapped buffer for example. A compression block that
    // only partly lies in the range is decompressed into a buffer of the thread first and the part is copied; every
    // other block is decompressed right where it belongs in dst.
    // Throws like subspan() if the range is outside the file or a chunk could not be read.
    void readInto(size_t offset, std::span<uint8_t> dst);

    void setOwner(AssetFileParser *ptr) { m_owner = ptr; }

  private:
    // Loads size bytes from the cursor on.
    bool load(size_t size) { return loadRange(m_cursor, size); }
    // Does not touch the cursor, so several threads can ask for ranges of the same file at once.
    bool loadRange(size_t offset, size_t size) {
      if (offset + size <= impl->loadedPrefix.load(std::memory_order_acquire)) [[likely]] {
        return true;
      }
      return impl->safeLoad(offset, size, m_buffer);
    }
    AssetFileParser *m_owner      = nullptr;
    uint64_t m_cursor             = 0;
    sc::MonotonicResource *m_storage           = nullptr;
    std::span<uint8_t> m_buffer{};
    GameFileImpl *impl = nullptr;
  };


} // namespace SAA
