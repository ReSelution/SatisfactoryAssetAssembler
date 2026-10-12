//
// Created by oleub on 15.01.26.
//

#include "file_reader.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <vector>


namespace SAA {
  std::span<const uint8_t> FileReader::getSpan(size_t offset, size_t length) const {
    assert(offset + length <= m_size);
    return {static_cast<const uint8_t *>(m_data) + offset, length};
  }

  bool FileReader::read(size_t offset, std::span<uint8_t> dst) const {
    assert(offset + dst.size() <= m_size);
#ifdef _WIN32
    size_t done = 0;
    while (done < dst.size()) {
      // With an offset in OVERLAPPED the read is positional, also on a synchronous handle.
      OVERLAPPED ov{};
      const uint64_t pos = offset + done;
      ov.Offset          = static_cast<DWORD>(pos);
      ov.OffsetHigh      = static_cast<DWORD>(pos >> 32);
      const DWORD want   = static_cast<DWORD>(std::min<size_t>(dst.size() - done, size_t{1} << 30));
      DWORD got          = 0;
      if (!ReadFile(m_fileHandle, dst.data() + done, want, &got, &ov) || got == 0) {
        return false;
      }
      done += got;
    }
    return true;
#else
    size_t done = 0;
    while (done < dst.size()) {
      const ssize_t n = ::pread(m_fd, dst.data() + done, dst.size() - done, static_cast<off_t>(offset + done));
      if (n < 0 && errno == EINTR) {
        continue;
      }
      if (n <= 0) {
        return false;
      }
      done += static_cast<size_t>(n);
    }
    return true;
#endif
  }

  std::span<const uint8_t> FileReader::readTemp(size_t offset, size_t length) const {
    thread_local std::vector<uint8_t> buffer;
    if (buffer.size() < length) {
      buffer.resize(length);
    }
    if (!read(offset, {buffer.data(), length})) {
      return {};
    }
    return {buffer.data(), length};
  }

  void FileReader::willNeed(size_t offset, size_t length) const {
    assert(offset + length <= m_size);
#ifndef _WIN32
    // Rounded out to 16 KiB: the neighbours come along and are usually read next. Measured on the cold crawl,
    // 4 KiB and 64 KiB+ were slower, and so was reading the rounded range with pread right away.
    constexpr size_t align = 16 * 1024;
    const size_t start     = offset & ~(align - 1);
    const size_t end       = std::min((offset + length + align - 1) & ~(align - 1), m_size);
    ::madvise(static_cast<uint8_t *>(m_data) + start, end - start, MADV_WILLNEED);
#else
    // Not announced on Windows. Under Wine a PrefetchVirtualMemory on the range saved ~14 ms cold and cost ~5 ms
    // warm, on a real Windows it is not measured.
    (void) offset;
    (void) length;
#endif
  }

  bool FileReader::openFile(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path))
      return false;
    m_size = std::filesystem::file_size(path);

#ifdef _WIN32
    // Windows / Wine Weg
    m_fileHandle = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, nullptr);
    if (m_fileHandle == INVALID_HANDLE_VALUE)
      return false;

    m_mappingHandle = CreateFileMappingW(m_fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!m_mappingHandle) {
      ::CloseHandle(m_fileHandle);
      return false;
    }
    m_data = MapViewOfFile(m_mappingHandle, FILE_MAP_READ, 0, 0, 0);
    WIN32_MEMORY_RANGE_ENTRY entry = { m_data, m_size };
    PrefetchVirtualMemory(GetCurrentProcess(), 1, &entry, 0);
#else
    // Linux Native
    m_fd = ::open(path.c_str(), O_RDONLY);
    if (m_fd == -1)
      return false;

    m_data = ::mmap(nullptr, m_size, PROT_READ, MAP_SHARED, m_fd, 0);

    if (m_data != MAP_FAILED) {
      // advice values are not flags, they cannot be combined with |
      ::madvise(m_data, m_size, MADV_HUGEPAGE);
    }
    if (m_data == MAP_FAILED) {
      ::close(m_fd);
      m_fd = -1;
      m_data = nullptr;
      return false;
    }
#endif
    return m_data != nullptr;
  }

  void FileReader::close() {
#ifdef _WIN32
    if (m_data)
      UnmapViewOfFile(m_data);
    if (m_mappingHandle)
      CloseHandle(m_mappingHandle);
    if (m_fileHandle != INVALID_HANDLE_VALUE)
      CloseHandle(m_fileHandle);
    m_data = nullptr;
    m_mappingHandle = nullptr;
    m_fileHandle = INVALID_HANDLE_VALUE;
#else
    if (m_data)
      munmap(m_data, m_size);
    if (m_fd != -1)
      ::close(m_fd);
    m_data = nullptr;
    m_fd = -1;
#endif
  }
} 
