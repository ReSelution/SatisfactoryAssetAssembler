#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <memory_resource>

#include "vfs_entry.hpp"

#include "io/f_io_chunk_id.hpp"

namespace SAP {
  constexpr size_t VFS_ARENA_SIZE = 1024 * 16;

  struct VfsStorage {
    std::vector<std::unique_ptr<std::pmr::monotonic_buffer_resource> > threadResources;

    VfsStorage(size_t bufferSize = VFS_ARENA_SIZE) {
      auto pool = std::make_unique<std::pmr::monotonic_buffer_resource>(bufferSize);
      threadResources.push_back(std::move(pool));
    }

    std::string_view persistString(std::string_view str) {
      char *mem = static_cast<char *>(threadResources.front()->allocate(str.size()));
      std::memcpy(mem, str.data(), str.size());
      return {mem, str.size()};
    }

    template<typename T, typename... Args>
    T *make(Args &&... args) {
      void *mem = threadResources.front()->allocate(sizeof(T), alignof(T));
      return new(mem) T(std::forward<Args>(args)...);
    }
  };

  class Vfs {
  public:
    Vfs getChild() { return Vfs(); }
    void mergeChild(Vfs &child);
    void reserve(size_t size);
    void mountEntry(IOStoreReader *parser, std::string_view pathStr, const FIoChunkId &chunkId);
    void mountEntry(PAKReader *parser, std::string_view pathStr, uint32_t index);
    void dumpToText(std::string_view path);
    [[nodiscard]] const UnifiedVfsEntry *readVfs(std::string_view path);
    const UnifiedVfsEntry *readPkg(const FPackageId &pkg);
    const std::unordered_map<uint64_t, const UnifiedVfsEntry *> &entries() { return m_vfsMap; }

  private:
    std::unordered_map<uint64_t, const UnifiedVfsEntry *> m_vfsMap;
    std::unordered_map<uint64_t, const UnifiedVfsEntry *> m_pkgMap;
    VfsStorage m_storage;
  };
} // namespace SAP
