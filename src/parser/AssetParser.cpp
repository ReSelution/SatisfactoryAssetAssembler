//
// Created by oleub on 07.12.25.
//

#include "AssetParser.hpp"

#include <filesystem>
#include "uasset_parser.hpp"
#include "vfs.hpp"

#include "IOStoreReader.hpp"
#include "Reader.hpp"

#include "crawler.hpp"

#include "logger/chaos_logger.hpp"
#include "threading/chaos_threading.hpp"

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


  std::shared_ptr<AssetFileParser> AssetParser::loadPackage(std::string_view path) {
    auto vfsEntry = m_vfs->readPkg(path);
    if (!vfsEntry) {
      // Logger::logError("[AssetParser]", "Unknown PKG : {}", pkg.Id);
      return nullptr;
    }
    return createParser(vfsEntry);
  }

  std::shared_ptr<AssetFileParser> AssetParser::loadPackage(FPackageId packageId) {
    auto vfsEntry = m_vfs->readPkg(packageId);
    if (!vfsEntry) {
      // Logger::logError("[AssetParser]", "Unknown PKG : {}", pkg.Id);
      return nullptr;
    }
    return createParser(vfsEntry);
  }

  void AssetParser::loadRes() {
    m_localizationManager.load();
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
    ZoneScopedN("CreateParser");
    if (auto shared = m_parserCache.getLocked(vfsEntry->hash)) {
      return shared;
    }

    switch (vfsEntry->getEntryType()) {
      case VFS_EntryType::IOVfsEntry: {
        const std::string_view fullPath = vfsEntry->packagePath;
        bool isUAsset = vfsEntry->isPkg;
        bool isUBulk = !isUAsset && fullPath.ends_with(".ubulk");
        if (!isUAsset && !isUBulk) {
          break;
        }

        auto entry = static_cast<const IOVfsEntry *>(vfsEntry);
        auto parser = std::make_shared<UAssetParser>(this, entry);
        const UnifiedVfsEntry *asset;
        const UnifiedVfsEntry *bulk;

        char pathBuf[512];
        if (fullPath.size() >= 512) [[unlikely]] return nullptr; // Safety first
        memcpy(pathBuf, fullPath.data(), fullPath.size());


        if (isUAsset) {
          asset = vfsEntry;
          memcpy(pathBuf + fullPath.size(), ".ubulk", 6);
          bulk = m_vfs->readVfs({pathBuf, fullPath.size() + 6});
        } else {
          bulk = vfsEntry;
          auto lastDot = fullPath.find_last_of('.');
          asset = m_vfs->readVfs({pathBuf, lastDot});
        }

        parser->setAssetData(asset);
        parser->setBulkData(bulk);

        parser->setReader(entry->owningReader);

        if (auto shared = m_parserCache.getLocked(vfsEntry->hash)) {
          return std::move(shared);
        }
        m_parserCache.insert(asset->hash, parser);
        if (bulk) {
          m_parserCache.insert(bulk->hash, parser);
        }

        return std::move(parser);
      }
      default:
        break;
    }
    return nullptr;
  }
} 
