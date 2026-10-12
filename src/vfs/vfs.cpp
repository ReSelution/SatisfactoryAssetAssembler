#include "vfs.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <new>
#include <ranges>
#include <vector>

#include "vfs_entry.hpp"

namespace SAA {
  void Vfs::mergeChild(Vfs &child) {
    std::lock_guard lock(m_mergeLock);
    m_vfsMap.insert(child.m_vfsMap.begin(), child.m_vfsMap.end());
    m_pkgMap.insert(child.m_pkgMap.begin(), child.m_pkgMap.end());
    child.m_vfsMap.clear();
    child.m_pkgMap.clear();
  }

  void Vfs::reserveIOStore(size_t size) {
    m_vfsMap.reserve(m_vfsMap.size() + size);
    m_pkgMap.reserve(m_pkgMap.size() + size);
  }

  void Vfs::reservePak(size_t size) { m_vfsMap.reserve(m_vfsMap.size() + size); }

  std::string_view Vfs::persistString(std::string_view str) {
    char *mem = static_cast<char *>(m_storage->allocate(str.size() + 1, alignof(char)));
    std::memcpy(mem, str.data(), str.size());
    mem[str.size()] = '\0';
    return {mem, str.size()};
  }

  void Vfs::mountEntry(IOStoreReader *parser, std::string_view pathStr, const FIoChunkId &chunkId) {
    auto persistenPath = persistString(pathStr);
    auto *entry = new (m_storage->allocate(sizeof(IOVfsEntry), alignof(IOVfsEntry)))
        IOVfsEntry(persistenPath, chunkId, parser);
    m_vfsMap.emplace(entry->hash, static_cast<SAA::UnifiedVfsEntry const *>(entry));
    m_pkgMap.emplace(chunkId.asPackageId().Id, static_cast<SAA::UnifiedVfsEntry const *>(entry));
  }

  void Vfs::mountEntry(PAKReader *parser, std::string_view pathStr, uint32_t index) {
    auto persistenPath = persistString(pathStr);
    auto *entry = new (m_storage->allocate(sizeof(PAKVfsEntry), alignof(PAKVfsEntry)))
        PAKVfsEntry(persistenPath, index, parser);
    m_vfsMap.emplace(entry->hash, static_cast<SAA::UnifiedVfsEntry const *>(entry));
  }

  void Vfs::dumpToText(std::string_view filepath) {
    std::filesystem::path tmpPath(filepath);
    create_directories(tmpPath.parent_path());
    auto fileStream = std::ofstream(tmpPath.c_str());

    auto pathView =
        m_vfsMap | std::ranges::views::values | std::views::transform([](const auto &entry) { return entry->pathStr; });
    std::vector<std::string_view> sortedPaths{pathView.begin(), pathView.end()};
    std::ranges::sort(sortedPaths);


    for (auto &path: sortedPaths) {
      fileStream << path << "\n";
    }
    fileStream.close();
  }

  const UnifiedVfsEntry *Vfs::readVfs(const std::string_view path) {
    auto iter = m_vfsMap.find(UnifiedVfsEntry::hashPath(UnifiedVfsEntry::cleanPathStatic(path)));
    if (iter == m_vfsMap.end()) {
      return nullptr;
    }
    return iter->second;
  }

  const UnifiedVfsEntry *Vfs::readPkg(const std::string_view path) {
    auto iter = m_vfsMap.find(UnifiedVfsEntry::hashPath(path));
    if (iter == m_vfsMap.end()) {
      return nullptr;
    }
    return iter->second;
  }

  const UnifiedVfsEntry *Vfs::readPkg(FPackageId pkgId) {
    auto iter = m_pkgMap.find(pkgId.Id);
    if (iter == m_pkgMap.end()) {
      return nullptr;
    }
    return iter->second;
  }
} // namespace SAA
