#include "games/satisfactory/SatisfactoryDataProvider.hpp"

#include <filesystem>
#include <fstream>

#include <memory>

#include <future>
#include <ranges>

#include "AssetParser.hpp"
#include "Decompressor.hpp"
#include "IOStoreReader.hpp"
#include "PAKReader.hpp"
#include "Reader.hpp"
#include "UsmapReader.hpp"
#include "crawler.hpp"
#include "io_global_data.hpp"
#include "logger.hpp"

#ifdef _WIN32
#include <windows.h>

#else
#include <dlfcn.h>

#endif

namespace SAP {
  static constexpr const char *LOGGER_PROVIDER = "[Provider]";
  static constexpr const char *LOGGER_OODLE = "[Oodle]";

  SatisfactoryDataProvider::SatisfactoryDataProvider(const std::filesystem::path &baseInstallPath) {
    Logger::Logger::getInstance().initLogger(LOGGER_PROVIDER);
    Logger::Logger::getInstance().initLogger(LOGGER_OODLE);

    Logger::logInfo(LOGGER_PROVIDER, "Starte Datei-Suche in {}", baseInstallPath.string());

    findAllParsableFiles(baseInstallPath);

    OodleLoader::loadOodle(baseInstallPath);
  }

  bool SatisfactoryDataProvider::load() {

    auto start = std::chrono::high_resolution_clock::now();

    auto usmapFuture = std::async(std::launch::async, [&] { return m_UsmapParser->parse(m_usmapData); });

    auto globalDataFuture = std::async(std::launch::async, [&] { return m_globalDataParser->parse(m_globalData); });

    std::vector<std::future<void>> parsableFutures;
    parsableFutures.reserve(m_Parsers.size());
    std::vector<Vfs> childVFS;
    childVFS.reserve(m_Parsers.size());

    for (const auto &val: m_Parsers | std::views::values) {
      auto *currentParser = val.get();
      auto &child = childVFS.emplace_back(m_vfs.getChild());
      parsableFutures.emplace_back(std::async(std::launch::async, [currentParser, &child] {
        currentParser->load();
        currentParser->mountVFS(&child);
      }));
    }

    for (auto &f: parsableFutures)
      f.wait();

    for (auto &child: childVFS) {
      m_vfs.mergeChild(child);
    }

    Logger::logInfo("[Provider]", "VFS erfolgreich erstellt. {} Dateien gemountet.", m_vfs.entries().size());
    m_vfs.dumpToText("output/filesEntry.txt");
    usmapFuture.get();
    globalDataFuture.get();
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[Provider]", "Loadedin {} ms", us.count() / 1000.0);
    return true;
  }

  void SatisfactoryDataProvider::crawl() {
    AssetParser parser(&m_vfs, &m_usmapData, &m_globalData);
    Crawler crawler;
    crawler.crawl(parser);
  }

  void SatisfactoryDataProvider::test() {
    AssetParser parser(&m_vfs, &m_usmapData, &m_globalData);
    parser.queueFile("FactoryGame/Content/FactoryGame/Recipes/Assembler/Recipe_IronPlateReinforced.uasset");

    // for (auto &[hash, entry]: m_vfs.entries()) {
    //   if (entry->path.ends_with(".uasset")) {
    //     parser.queueFile(entry->path);
    //   }
    // }

    parser.processQueue();
  }

  void SatisfactoryDataProvider::findAllParsableFiles(const std::filesystem::path &folder) {
    auto start = std::chrono::high_resolution_clock::now();

    if (!std::filesystem::exists(folder) || !std::filesystem::is_directory(folder)) {
      Logger::logWarn(LOGGER_PROVIDER, "Pfad {} ist ungültig.", folder.string());
      return;
    }

    std::vector<std::string> subFolders = {"FactoryGame/Content/Paks", "CommunityResources"};
    for (const auto &sub: subFolders) {
      std::filesystem::path searchPath = folder / sub;

      if (!std::filesystem::exists(searchPath))
        continue;

      for (const auto &entry: std::filesystem::directory_iterator(searchPath)) {
        if (!entry.is_regular_file()) {
          continue;
        }
        if (entry.is_symlink()) {
          continue;
        }
        auto path = entry.path();
        auto ext = path.extension().string();
        auto basename = path.string();

        if (ext == ".pak") {
          m_Parsers[basename] = std::make_unique<PAKReader>(path);
        } else if (ext == ".utoc") {
          std::filesystem::path ucasPath = path;
          ucasPath.replace_extension(".ucas");

          if (std::filesystem::exists(ucasPath)) {
            if (path.filename() == "global.utoc") {
              m_globalDataParser = std::make_unique<IOGlobalData>(path, ucasPath);
            } else {
              m_Parsers[basename] = std::make_unique<IOStoreReader>(path, ucasPath);
            }
          } else {
            Logger::logWarn(LOGGER_PROVIDER, "UTOC gefunden, aber keine passende UCAS: {}", path.string());
          }
        } else if (ext == ".usmap") {
          m_UsmapParser = std::make_unique<UsmapReader>(path);
        }
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[Files]", "{} ms", us.count() / 1000.0);
  }

  SatisfactoryDataProvider::~SatisfactoryDataProvider() {}
} // namespace SAP
