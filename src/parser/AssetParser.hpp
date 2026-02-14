//
// Created by oleub on 07.12.25.
//

#pragma once
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

#include "loc_manager.hpp"
#include "shared_cache.hpp"


namespace SAP {
  struct GlobalData;
  class Crawler;
  struct FPackageId;
  class Reader;
  class AssetFileParser;
} // namespace SAP

namespace SAP {
  struct AssetParseResult;
  class Vfs;
  struct FUsmapMappingData;

  class AssetParser {
  public:
    explicit AssetParser(Vfs *vfs, const FUsmapMappingData *mappings, const GlobalData *globalData);
    ~AssetParser();

    void queueFile(std::string_view path);
    void processQueue(Crawler *crawler = nullptr, uint32_t threadCount = 0);
    std::shared_ptr<AssetFileParser> loadPackage(const FPackageId &pkg);
    Vfs *getVfs() const { return m_vfs; };

  private:
    friend class AssetFileParser;
    [[nodiscard]] const FUsmapMappingData *mappings() const { return m_mappings; }
    [[nodiscard]] const GlobalData *globalData() const { return m_globalData; }
    void workerThread(const std::function<void(std::shared_ptr<AssetFileParser> &)> &onParsed);
    void parseAsset(std::string_view path, const std::function<void(std::shared_ptr<AssetFileParser> &)> &onParsed);

    std::shared_ptr<AssetFileParser> createParser(const UnifiedVfsEntry *vfsEntry);
    Vfs *m_vfs;
    const FUsmapMappingData *m_mappings;
    const GlobalData *m_globalData;

    LocManager m_localizationManager;

    std::queue<std::string_view> m_JobQueue;
    std::mutex m_QueueMutex;
    std::vector<std::thread> m_ThreadPool;
    ShardedCache<uint64_t, std::weak_ptr<AssetFileParser>> m_parserCache;
    std::shared_mutex m_cacheMutex;
  };
} // namespace SAP
