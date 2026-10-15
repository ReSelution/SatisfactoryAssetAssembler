#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>

#include "ankerl/unordered_dense.h"

#include "io/f_io_chunk_id.hpp"
#include "sc/hash/hash.hpp"
#include "sc/memory/huge_page_resource.hpp"
#include "sc/memory/monotonic_resource.hpp"
#include "vfs_entry.hpp"

namespace SAA {
  class Vfs {
    using denseMap = ankerl::unordered_dense::map<uint64_t, const UnifiedVfsEntry *, sc::TransparentHash>;

  public:
    Vfs() :
        m_ownStorage(std::make_unique<sc::MonotonicResource>(sc::HugePageResource::HUGE_PAGE_SIZE,
                                                             sc::HugePageResource::get())),
        m_storage(m_ownStorage.get()) {}

    // A Vfs that mounts into the storage of this one, so that several threads can each fill their own maps. Its
    // entries only reach this Vfs with mergeChild(), and it must not outlive this Vfs.
    [[nodiscard]] Vfs makeChild() const { return Vfs(m_storage); }

    // Takes over the entries of a child. The entries themselves already lie in the shared storage.
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
    explicit Vfs(sc::MonotonicResource *parentStorage) : m_storage(parentStorage) {}

    std::string_view persistString(std::string_view str);

    static inline std::mutex m_mergeLock{};
    denseMap m_vfsMap;
    denseMap m_pkgMap;

    // Paths and entries of this Vfs and of its children; a child has no storage of its own. Allocating is safe
    // from several threads, everything is given back at once. The entries own nothing and get no destructor call.
    // Behind a pointer because the resource cannot be moved.
    // Not an sc::Heap: measured, mounting the utoc of 1009 took 7.2-9.7 ms with it instead of 5.2-6.8 ms.
    std::unique_ptr<sc::MonotonicResource> m_ownStorage;
    sc::MonotonicResource *m_storage;
  };
} // namespace SAA
