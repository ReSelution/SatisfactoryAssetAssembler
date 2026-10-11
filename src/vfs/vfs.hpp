#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <string_view>
#include <utility>
#include <vector>

#include "ankerl/unordered_dense.h"

#include "huge_page_resource.hpp"

#include "io/f_io_chunk_id.hpp"
#include "sc/hash/hash.hpp"
#include "vfs_entry.hpp"

namespace SAA {
  constexpr size_t MAX_PATH_SIZE     = 256 * sizeof(char);
  constexpr size_t IOSTORE_ENTR_SIZE = MAX_PATH_SIZE + sizeof(IOVfsEntry);
  constexpr size_t PAK_ENTR_SIZE     = MAX_PATH_SIZE + sizeof(PAKVfsEntry);

  // TODO: Das Spiel mit den Storages durch sc::MonotonicResource ersetzen. Im Moment legt jeder Reader ueber
  // reserveIOStore()/
  // reservePAK() einen eigenen monotonic_buffer_resource an (vorab auf count * (256 + sizeof(Entry)) Bytes
  // geschaetzt), fuellt ihn in einem Kind-Vfs und mergeChild() schiebt die Resources danach unter m_mergeLock in das
  // Eltern-Vfs. Das gibt es nur, weil monotonic_buffer_resource nicht threadsicher ist.
  // Bei sc::MonotonicResource sind allocate()/make() von mehreren Threads gleichzeitig sicher, und alles wird auf
  // einmal freigegeben. Eine einzige Resource im Vfs wuerde also reichen: kein index, keine Groessenschaetzung, kein
  // Verschieben der Resources beim Mergen. Offen bleibt dann nur das Zusammenfuehren der Maps. sc::MonotonicResource
  // ist nicht kopier- oder verschiebbar, Vfs::getChild() gibt ein Vfs aber per Wert zurueck; die Kind-Vfs braeuchten
  // dann einen Zeiger auf die Resource des Eltern-Vfs statt einer eigenen.
  struct VfsStorage {
    std::vector<std::unique_ptr<std::pmr::monotonic_buffer_resource>> threadResources;
    size_t index = std::numeric_limits<size_t>::max();

    void reserveIOStore(size_t count) {
      auto pool = std::make_unique<std::pmr::monotonic_buffer_resource>(IOSTORE_ENTR_SIZE * count, HugePageResource::get());
      threadResources.push_back(std::move(pool));
      index = threadResources.size() - 1;
    }

    void reservePAK(size_t count) {
      auto pool = std::make_unique<std::pmr::monotonic_buffer_resource>(PAK_ENTR_SIZE * count, HugePageResource::get());
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
    T *make(Args &&...args) {
      assert(index >= 0 && index < threadResources.size() && "CALL Reserve first");
      void *mem = threadResources[index]->allocate(sizeof(T), alignof(T));
      return new (mem) T(std::forward<Args>(args)...);
    }
  };

  class Vfs {
    using denseMap = ankerl::unordered_dense::map<uint64_t, const UnifiedVfsEntry *, sc::IdentityHash>;

  public:
    static Vfs getChild() { return {}; }

    void mergeChild(Vfs &child);

    void reserveIOStore(size_t size);

    void reservePak(size_t size);

    void mountEntry(IOStoreReader *parser, std::string_view pathStr, const FIoChunkId &chunkId);

    void mountEntry(PAKReader *parser, std::string_view pathStr, uint32_t index);

    void dumpToText(std::string_view path);

    [[nodiscard]] const UnifiedVfsEntry *readVfs(std::string_view path);

    const UnifiedVfsEntry *readPkg(std::string_view path);

    const UnifiedVfsEntry *readPkg(FPackageId pkgId);

    const denseMap &entries() { return m_vfsMap; }

  private:
    static inline std::mutex m_mergeLock{};
    denseMap m_vfsMap;
    denseMap m_pkgMap;

    VfsStorage m_storage{};
  };
} // namespace SAA
