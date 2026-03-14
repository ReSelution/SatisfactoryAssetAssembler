#pragma once

#include <cassert>
#include <filesystem>
#include <memory_resource>
#include <unordered_map>
#include <vector>

#include "vfs_entry.hpp"
#include "core/hash.hpp"

#include "io/f_io_chunk_id.hpp"

namespace SAA {
  constexpr size_t MAX_PATH_SIZE = 256 * sizeof(char);
  constexpr size_t IOSTORE_ENTR_SIZE = MAX_PATH_SIZE + sizeof(IOVfsEntry);
  constexpr size_t PAK_ENTR_SIZE = MAX_PATH_SIZE + sizeof(PAKVfsEntry);

  struct VfsStorage {
    std::vector<std::unique_ptr<std::pmr::monotonic_buffer_resource> > threadResources;
    size_t index = -1;

    void reserveIOStore(size_t count) {
      auto pool = std::make_unique<std::pmr::monotonic_buffer_resource>(IOSTORE_ENTR_SIZE * count);
      threadResources.push_back(std::move(pool));
      index = threadResources.size() - 1;
    }

    void reservePAK(size_t count) {
      auto pool = std::make_unique<std::pmr::monotonic_buffer_resource>(PAK_ENTR_SIZE * count);
      threadResources.push_back(std::move(pool));
      index = threadResources.size() - 1;
    }

    std::string_view persistString(std::string_view str) {
      assert(index >= 0 && index < threadResources.size() && "CALL Reserve first");
      char *mem = static_cast<char *>(threadResources[index]->allocate(str.size() + 1));
      std::memcpy(mem, str.data(), str.size());
      mem[str.size()] = '\0';
      return {mem, str.size()};
    }

    template<typename T, typename... Args>
    T *make(Args &&... args) {
      assert(index >= 0 && index < threadResources.size() && "CALL Reserve first");
      void *mem = threadResources[index]->allocate(sizeof(T), alignof(T));
      return new(mem) T(std::forward<Args>(args)...);
    }
  };

  class Vfs {
  public:
    Vfs getChild() { return Vfs(); }

    void mergeChild(Vfs &child);

    void reserveIOStore(size_t size);

    void reservePak(size_t size);

    void mountEntry(IOStoreReader *parser, std::string_view pathStr, const FIoChunkId &chunkId);

    void mountEntry(PAKReader *parser, std::string_view pathStr, uint32_t index);

    void dumpToText(std::string_view path);

    [[nodiscard]] const UnifiedVfsEntry *readVfs(std::string_view path);

    const UnifiedVfsEntry *readPkg(const FPackageId &pkg);

    const std::unordered_map<uint64_t, const UnifiedVfsEntry *, IdentityHash> &entries() { return m_vfsMap; }

  private:
    std::unordered_map<uint64_t, const UnifiedVfsEntry *, IdentityHash> m_vfsMap;
    std::unordered_map<uint64_t, const UnifiedVfsEntry *, IdentityHash> m_pkgMap;
    VfsStorage m_storage{};
  };
} // namespace SAP
