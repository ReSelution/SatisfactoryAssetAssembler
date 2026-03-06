//
// Created by oleub on 07.12.25.
//

#include "AssetParser.hpp"

#include <filesystem>
#include "uasset_parser.hpp"
#include "vfs.hpp"

#include "IOStoreReader.hpp"
#include "Reader.hpp"
#include "ThroughputStats.hpp"

#include "crawler.hpp"
#include "heap_pool.hpp"
#include "puplic/arena/registery.hpp"

#include "logger/chaos_logger.hpp"
LOG_ALIAS(APLog, "SAA", "AssetParser")

namespace SAA {
  AssetParser::AssetParser(Vfs *vfs, const FUsmapMappingData *mappings, const GlobalData *globalData) : m_vfs(vfs),
                                                                                                        m_mappings(
                                                                                                          mappings),
                                                                                                        m_globalData(
                                                                                                          globalData),
                                                                                                        m_localizationManager(
                                                                                                          this) {
  }

  void AssetParser::queueFile(std::string_view path) {
    {
      std::lock_guard lock(m_QueueMutex);
      m_JobQueue.emplace(path);
    }
  }

  void AssetParser::processQueue(Crawler *crawler, uint32_t threadCount) {
    m_localizationManager.setCulture("de");
    m_localizationManager.load();
    if (threadCount == 0) {
      threadCount = std::max(1u, (std::thread::hardware_concurrency() - 1));
    }
    HeapPool::init(threadCount * 4, 8 * 1024);
    Registry::init(2048 * 1024);
    m_parserCache.reserve(m_vfs->entries().size());
    APLog::info("Start Parsing of {} files on {} Threads with {} KB cache Size  ",
                m_JobQueue.size(), threadCount, HeapPool::getBufferSize() / 1024.0);
    std::vector<Crawler> crawlers;
    crawlers.reserve(threadCount);

    auto t = APLog::time("[AssetParser]", "Parsing finished: {} ms");
    ThroughputStats::start();
    for (uint32_t i = 0; i < threadCount; ++i) {
      if (crawler) {
        auto &child = crawlers.emplace_back(crawler->getChild());
        m_ThreadPool.emplace_back(&AssetParser::workerThread, this,
                                  [&child](std::shared_ptr<AssetFileParser> &parser) { child.onParsed(parser); });
      } else {
        m_ThreadPool.emplace_back(&AssetParser::workerThread, this,
                                  [](std::shared_ptr<AssetFileParser> &parser) {
                                    parser->loadExports();
                                  });
      }
    }

    for (uint32_t i = 0; i < threadCount; ++i) {
      if (m_ThreadPool[i].joinable()) {
        m_ThreadPool[i].join();
        if (crawler) {
          crawler->mergeWithChild(crawlers[i]);
        }
      }
    }

    m_ThreadPool.clear();


    APLog::info(
      "Durchsatz: Decompresed: {:.2f} MB/s Compressed: {:.2f} MB/s (Decompressed: {:.2f} GB, On Disk: {:.2f} GB)",
      ThroughputStats::getMBPerSecondDecompressed(),
      ThroughputStats::getMBPerSecondCompressed(),
      ThroughputStats::totalDecompressedBytes.load() / (1024.0 * 1024.0 * 1024.0),
      ThroughputStats::totalCompressedBytes.load() / (1024.0 * 1024.0 * 1024.0));
  }

  std::shared_ptr<AssetFileParser> AssetParser::loadPackage(const FPackageId &pkg) {
    auto vfsEntry = m_vfs->readPkg(pkg);
    if (!vfsEntry) {
      // Logger::logError("[AssetParser]", "Unknown PKG : {}", pkg.Id);
      return nullptr;
    }
    return createParser(vfsEntry);
  }

  void AssetParser::workerThread(const std::function<void(std::shared_ptr<AssetFileParser> &)> &onParsed) {
    while (true) {
      std::string path;
      {
        std::lock_guard lock(m_QueueMutex);
        if (m_JobQueue.empty()) {
          return; // keine Jobs mehr, Thread beendet sich
        }
        path = m_JobQueue.front();
        m_JobQueue.pop();
      }
      parseAsset(path, onParsed);
    }
  }

  void AssetParser::parseAsset(std::string_view path,
                               const std::function<void(std::shared_ptr<AssetFileParser> &)> &onParsed) {
    const auto &vfsEntry = m_vfs->readVfs(path);
    if (!vfsEntry) {
      return;
    }

    auto parser = createParser(vfsEntry);
    if (!parser) {
      APLog::info("[AssetParser]", "Unknown file : {}", path);
      return;
    }

    parser->safeParse();
    onParsed(parser);
  }

  std::shared_ptr<AssetFileParser> AssetParser::createParser(const UnifiedVfsEntry *vfsEntry) {
    if (auto shared = m_parserCache.get(vfsEntry->hash).lock()) {
      return shared;
    }

    switch (vfsEntry->type) {
      case VFS_EntryType::IOVfsEntry: {
        const std::string_view fullPath = vfsEntry->path;
        bool isUAsset = fullPath.ends_with(".uasset");
        bool isUBulk = !isUAsset && fullPath.ends_with(".ubulk");
        if (!isUAsset && !isUBulk) {
          break;
        }

        auto entry = static_cast<const IOVfsEntry *>(vfsEntry);

        auto parser = std::make_shared<UAssetParser>(this, entry);

        const UnifiedVfsEntry *asset;
        const UnifiedVfsEntry *bulk;

        char pathBuf[512];
        if (fullPath.size() >= 512)
          return nullptr; // Safety first
        memcpy(pathBuf, fullPath.data(), fullPath.size());
        auto lastDot = fullPath.find_last_of('.');

        if (isUAsset) {
          asset = vfsEntry;
          memcpy(pathBuf + lastDot, ".ubulk", 6);
          bulk = m_vfs->readVfs({pathBuf, lastDot + 6});
        } else {
          bulk = vfsEntry;
          memcpy(pathBuf + lastDot, ".uasset", 7);
          asset = m_vfs->readVfs({pathBuf, lastDot + 7});
        }

        parser->setAssetData(asset);
        parser->setBulkData(bulk);

        parser->setReader(entry->owningReader);

        if (auto shared = m_parserCache.get(vfsEntry->hash).lock()) {
          return shared;
        }
        m_parserCache.insert(asset->hash, parser);
        if (bulk) {
          m_parserCache.insert(bulk->hash, parser);
        }

        return parser;
      }
      default:
        break;
    }
    return nullptr;
  }

  AssetParser::~AssetParser() {
  }
} // namespace SAP
