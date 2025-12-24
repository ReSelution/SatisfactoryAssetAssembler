//
// Created by oleub on 07.12.25.
//

#pragma once
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "io_global_data.hpp"

namespace SAP {
  struct FPackageId;
  class Reader;
  class AssetFileParser;
}

namespace SAP {
  struct AssetParseResult;
  class Vfs;
  struct FUsmapMappingData;

  class AssetParser {
  public:
    explicit AssetParser(const Vfs *vfs, const FUsmapMappingData *mappings, const GlobalData *globalData);
    ~AssetParser();

    void queueFile(const std::filesystem::path &path);
    void processQueue(const std::function<void(const AssetParseResult &)> &onAssetParsed, uint32_t threadCount = 0);
    std::unique_ptr<AssetFileParser> loadPackage(const FPackageId &pkg);


  private:
    friend class AssetFileParser;
    const FUsmapMappingData* mappings() const {return m_mappings;}
    const GlobalData *globalData() const {return m_globalData;}
    void workerThread(const std::function<void(const AssetParseResult &)> &onAssetParsed);
    void parseAsset
    (const std::function<void(const AssetParseResult &)> &onAssetParsed, const std::filesystem::path &assetPath);

    std::unique_ptr<AssetFileParser> createParser(const std::filesystem::path &assetPath,   Reader *dataParser);
    const Vfs *m_vfs;
    const FUsmapMappingData *m_mappings;
    const GlobalData *m_globalData;

    std::queue<std::filesystem::path> m_JobQueue;
    std::mutex m_QueueMutex;
    std::vector<std::thread> m_ThreadPool;
  };
}

