//
// Created by oleub on 07.12.25.
//

#include "AssetParser.hpp"

#include "uasset_parser.hpp"
#include "logger.hpp"
#include "vfs.hpp"
#include <filesystem>

#include "../assets/ue/object_factory.hpp"
#include "IOStoreReader.hpp"
#include "Reader.hpp"

namespace SAP {
  AssetParser::AssetParser(const Vfs *vfs, const FUsmapMappingData *mappings, const GlobalData *globalData) : m_vfs(vfs), m_mappings(mappings), m_globalData(globalData) {
  }

  void AssetParser::queueFile(const std::filesystem::path &path) {
    {
      std::lock_guard lock(m_QueueMutex);
      m_JobQueue.emplace(path);
    }
  }

  void AssetParser::processQueue
  (const std::function<void(const AssetParseResult &)> &onAssetParsed, uint32_t threadCount) {
    if (threadCount == 0) { threadCount = std::max(1u, std::thread::hardware_concurrency() - 2); }

    for (uint32_t i = 0; i < threadCount; ++i) {
      m_ThreadPool.emplace_back(&AssetParser::workerThread, this, onAssetParsed);
    }

    for (auto &thread: m_ThreadPool) { if (thread.joinable()) { thread.join(); } }

    m_ThreadPool.clear();
  }

  std::unique_ptr<AssetFileParser> AssetParser::loadPackage(const FPackageId &pkg) {
    UnifiedVfsEntry vfsEntry = m_vfs->readPkg(pkg);
    auto assetData = vfsEntry.owningParser->readChunk(vfsEntry.chunkId);
    auto bulkPath = vfsEntry.path;
    bulkPath.replace_extension(".ubulk");
    auto bulkData = m_vfs->readFile(bulkPath);
    auto parser =  createParser(vfsEntry.path, vfsEntry.owningParser);
    if (!parser) {
      Logger::logError("[AssetParser]", "Unknown file : {}", vfsEntry.path.string());
      return nullptr;
    }
    //AssetParseResult result = parser->parse(assetData, bulkData);
    return parser;
  }

  void AssetParser::workerThread(const std::function<void(const AssetParseResult &)> &onAssetParsed) {
    while (true) {
      std::string path;
      {
        std::lock_guard lock(m_QueueMutex);
        if (m_JobQueue.empty()) {
          return; // keine Jobs mehr, Thread beendet sich
        }
        path = std::move(m_JobQueue.front());
        m_JobQueue.pop();
      }
      parseAsset(onAssetParsed, path);
    }
  }

  void AssetParser::parseAsset
  (const std::function<void(const AssetParseResult &)> &onAssetParsed, const std::filesystem::path &assetPath) {
    if (assetPath.empty()) { return; }

    const auto vfsEntry = m_vfs->readVfs(assetPath);
    if (vfsEntry == UnifiedVfsEntry{}) { return; }

    auto parser = createParser(assetPath, vfsEntry.owningParser);
    if (!parser) {
      Logger::logInfo("[AssetParser]", "Unknown file : {}", assetPath.filename().string());
      return;
    }

    auto assetData = vfsEntry.owningParser->readChunk(vfsEntry.chunkId);

    auto bulkPath = assetPath;
    bulkPath.replace_extension(".ubulk");
    auto bulkData = m_vfs->readFile(bulkPath);

    Logger::logInfo("[AssetParser]", "parsing asset: {}", assetPath.string());
    AssetParseResult result = parser->parse(assetData, bulkData);
  }

  std::unique_ptr<AssetFileParser> AssetParser::createParser
  (const std::filesystem::path &assetPath, Reader *dataParser) {
    auto ext = assetPath.extension();
    std::unique_ptr<AssetFileParser> parser;
    switch (dataParser->type()) {
      case Reader::Type::IO_STORE:
        if (ext == ".uasset") {
          auto path = assetPath;
          path.replace_extension();
          parser = std::make_unique<UAssetParser>
              ( this,path.string(),&reinterpret_cast<IOStoreReader *>(dataParser)->containerHeader());
        }
        break;
      case Reader::Type::PAK:
        break;
    }
    return parser;
  }

  AssetParser::~AssetParser() {}
}
