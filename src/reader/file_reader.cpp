//
// Created by oleub on 15.01.26.
//

#include "file_reader.hpp"

#include <cassert>


namespace SAA {
  std::span<const uint8_t> FileReader::getSpan(size_t offset, size_t length) const {
    assert(offset + length <= m_size);
    return {static_cast<const uint8_t *>(m_data) + offset, length};
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

    m_data = ::mmap(nullptr, m_size, PROT_READ, MAP_PRIVATE, m_fd, 0);

    if (m_data != MAP_FAILED) {
      ::madvise(m_data, m_size, MADV_WILLNEED | MADV_SEQUENTIAL | MADV_HUGEPAGE);
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
} // namespace SAP
