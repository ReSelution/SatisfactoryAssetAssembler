//
// Created by oleub on 15.01.26.
//

#pragma once

#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <span>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace SAP {
  class FileReader {
  public:
    FileReader(const std::filesystem::path &path) { openFile(path); };
    ~FileReader() { close(); }

    [[nodiscard]] std::span<const uint8_t> getSpan(size_t offset = 0, size_t length = 0) const;
    bool openFile(const std::filesystem::path &path);
    [[nodiscard]] size_t size() const { return m_size; };
    void close();

  private:
    void *m_data = nullptr;
    size_t m_size = 0;
#ifdef _WIN32
    HANDLE m_fileHandle = INVALID_HANDLE_VALUE;
    HANDLE m_mappingHandle = nullptr;
#else
    int m_fd = -1;
#endif
  };
} // namespace SAP
