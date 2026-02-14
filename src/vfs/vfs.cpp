#include "vfs.hpp"

#include <fstream>

#include "logger.hpp"
#include "vfs_entry.hpp"

namespace SAP {

  void Vfs::mergeChild(Vfs &child) {

    m_vfsMap.merge(child.m_vfsMap);
    m_pkgMap.merge(child.m_pkgMap);
    std::ranges::move(child.m_storage.threadResources, std::back_inserter(m_storage.threadResources));
    child.m_storage.threadResources.clear();
  }

  void Vfs::reserveIOStore(size_t size) {
    m_vfsMap.reserve(m_vfsMap.size() + size);
    m_pkgMap.reserve(m_vfsMap.size() + size);
    m_storage.reserveIOStore(size);
  }
  void Vfs::reservePak(size_t size) {
    m_vfsMap.reserve(m_vfsMap.size() + size);
    m_storage.reservePAK(size);

  }


  void Vfs::mountEntry(IOStoreReader *parser, std::string_view pathStr, const FIoChunkId &chunkId) {
    auto persistenPath = m_storage.persistString(pathStr);

    auto entry = m_storage.make<IOVfsEntry>(persistenPath, chunkId,parser);
    m_vfsMap.emplace(entry->hash, entry);
    m_pkgMap.emplace(chunkId.asPackageId().Id, entry);
  }

  void Vfs::mountEntry(PAKReader *parser,  std::string_view pathStr, uint32_t index) {
    auto persistenPath = m_storage.persistString(pathStr);
    auto entry = m_storage.make<PAKVfsEntry>(persistenPath, index, parser);
    m_vfsMap.emplace(entry->hash, std::move(entry));
  }

  void Vfs::dumpToText(std::string_view path) {
    auto fileStream = std::ofstream(std::string(path));
    for (auto &entry: m_vfsMap) {
      fileStream <<  entry.second->path << "\n";
    }
    fileStream.close();
  }

  const UnifiedVfsEntry *Vfs::readVfs(const std::string_view path) {
    auto iter = m_vfsMap.find(UnifiedVfsEntry::hashPath(path));
    if (iter == m_vfsMap.end()) {
      return nullptr;
    }
    return iter->second;
  }

  const UnifiedVfsEntry *Vfs::readPkg(const FPackageId &pkg) {
    auto iter = m_pkgMap.find(pkg.Id);
    if (iter == m_pkgMap.end()) {
      return nullptr;
    }
    return iter->second;
  }
} // namespace SAP
