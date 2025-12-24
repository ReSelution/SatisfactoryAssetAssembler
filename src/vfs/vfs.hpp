#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "io/f_io_chunk_id.hpp"

namespace SAP {
  class Reader;

  struct UnifiedVfsEntry {
    std::filesystem::path path;
    FIoChunkId chunkId;
    Reader *owningParser;

    bool operator==(const UnifiedVfsEntry &other) const {
      return this->chunkId == other.chunkId && this->owningParser == other.owningParser;
    }

    bool operator!=(const UnifiedVfsEntry &other) const { return !this->operator==(other); }
  };

  class Vfs {
  public:
    void reserve(size_t size);
    void mountEntry(Reader *parser, const std::filesystem::path &path, FIoChunkId chunkId);
    [[nodiscard]] std::vector<uint8_t> readFile(const std::string &path) const;
    [[nodiscard]] UnifiedVfsEntry readVfs(const std::string &path) const;
    UnifiedVfsEntry readPkg(const FPackageId &pkg) const;

  private:
    std::unordered_map<std::filesystem::path, UnifiedVfsEntry> m_VfsMap;
    std::unordered_map<FPackageId, UnifiedVfsEntry> m_packageMap;
  };
} // namespace SAP
