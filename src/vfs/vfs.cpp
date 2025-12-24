#include "vfs.hpp"

#include "Reader.hpp"
#include "logger.hpp"

namespace SAP {
  void Vfs::reserve(size_t size) { m_VfsMap.reserve(m_VfsMap.size() + size); }

  void Vfs::mountEntry(Reader *parser, const std::filesystem::path &path, FIoChunkId chunkId) {
    m_VfsMap.emplace(path, UnifiedVfsEntry{.path = path, .chunkId = chunkId, .owningParser = parser});
    m_packageMap.emplace
        (chunkId.asPackageId(), UnifiedVfsEntry{.path = path, .chunkId = chunkId, .owningParser = parser});
  }

  std::vector<uint8_t> Vfs::readFile(const std::string &path) const {
    auto iter = m_VfsMap.find(path);

    if (iter == m_VfsMap.end()) { return {}; }

    const UnifiedVfsEntry &entry = iter->second;
    return entry.owningParser->readChunk(entry.chunkId);
  }

  UnifiedVfsEntry Vfs::readVfs(const std::string &path) const {
    auto iter = m_VfsMap.find(path);
    if (iter == m_VfsMap.end()) { return {}; }
    return iter->second;
  }

  UnifiedVfsEntry Vfs::readPkg(const FPackageId &pkg) const {
    auto iter = m_packageMap.find(pkg);
    if (iter == m_packageMap.end()) { return {}; }
    return iter->second;
  }
} // namespace SAP
