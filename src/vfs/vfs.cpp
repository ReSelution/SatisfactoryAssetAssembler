#include "vfs.hpp"

#include <algorithm>
#include <fstream>
#include <ranges>

#include "vfs_entry.hpp"

namespace SAA {
    void Vfs::mergeChild(Vfs &child) {
        std::lock_guard lock(m_mergeLock);
        m_vfsMap.insert(child.m_vfsMap.begin(), child.m_vfsMap.end());
        m_pkgMap.insert(child.m_pkgMap.begin(), child.m_pkgMap.end());
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

        auto entry = m_storage.make<IOVfsEntry>(persistenPath, chunkId, parser);
        m_vfsMap.emplace(entry->hash, entry);
        m_pkgMap.emplace(chunkId.asPackageId().Id, entry);
    }

    void Vfs::mountEntry(PAKReader *parser, std::string_view pathStr, uint32_t index) {
        auto persistenPath = m_storage.persistString(pathStr);
        auto entry = m_storage.make<PAKVfsEntry>(persistenPath, index, parser);
        m_vfsMap.emplace(entry->hash, std::move(entry));
    }

    void Vfs::dumpToText(std::string_view filepath) {
        std::filesystem::path tmpPath(filepath);
        create_directories(tmpPath.parent_path());
        auto fileStream = std::ofstream(std::string(filepath));

        auto pathView = m_vfsMap | std::ranges::views::values
                        | std::views::transform([](const auto &entry) {
            return entry->path;
        });
        std::vector<std::string_view> sortedPaths{pathView.begin(), pathView.end()};
        std::ranges::sort(sortedPaths);


        for (auto &path: sortedPaths) {
            fileStream << path << "\n";
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
} 
