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
#include "PropertyTracker.hpp"

#include "logger/chaos_logger.hpp"
#include "puplic/arena/registery.hpp"
#include "threading/chaos_threading.hpp"

LOG_ALIAS(ProviderLog, "SAA", "Provider")

namespace SAA {
  SatisfactoryDataProvider::SatisfactoryDataProvider(const std::filesystem::path &baseInstallPath) {
    ProviderLog::info("Searching in {}", baseInstallPath.string());

    findAllParsableFiles(baseInstallPath);

    OodleLoader::loadOodle(baseInstallPath);
  }

  bool SatisfactoryDataProvider::load() {
    auto t = ProviderLog::time("Loaded in {}");
#ifdef NDEBUG
    std::vector<std::future<void> > parsableFutures;
    parsableFutures.reserve(m_Parsers.size() + 2);
    parsableFutures.emplace_back(SC::ChaosThreading::enqueue([&](int id) { m_UsmapParser->parse(m_usmapData); }));
    parsableFutures.emplace_back(SC::ChaosThreading::enqueue([&](int id) {
      m_globalDataParser->parse(m_globalData);
    }));


    std::vector<Vfs> childVFS;
    childVFS.reserve(m_Parsers.size());
    for (int i = 0; i < SC::ChaosThreading::getNumThreads(); i++) {
      childVFS.emplace_back(SAA::Vfs::getChild());
    }

    for (const auto &val: m_Parsers | std::views::values) {
      auto *currentParser = val.get();
      parsableFutures.emplace_back(SC::ChaosThreading::enqueue([&, currentParser](int id) {
        auto &child = childVFS[id];
        currentParser->load();
        currentParser->mountVFS(&m_vfs);
        m_vfs.mergeChild(child);

      }));
    }

    for (auto &f: parsableFutures)
      f.wait();
#else
    // --- DEBUG MODUS: Sequenziell ---
    ProviderLog::debug("Running in Debug mode: Sequential load.");

    // 1. Basis-Parser direkt ausführen
    m_UsmapParser->parse(m_usmapData);
    m_globalDataParser->parse(m_globalData);

    // 2. Ein einzelnes Child-VFS für den aktuellen Thread
    auto child = SAA::Vfs::getChild();

    for (const auto &val : m_Parsers | std::views::values) {
      auto *currentParser = val.get();

      currentParser->load();
      currentParser->mountVFS(&m_vfs);

      // Da wir sequenziell arbeiten, können wir direkt mergen
      m_vfs.mergeChild(child);
    }
#endif

//    SC::ChaosThreading::enqueue<SC::ChaosThreading::Priority::High>([&]( int id) {
//      m_vfs.dumpToText("output/files.txt");
//    });
    ProviderLog::info("VFS erfolgreich erstellt. {} Dateien gemountet.", m_vfs.entries().size());
    return true;
  }

  void SatisfactoryDataProvider::crawl() {
    AssetParser parser(&m_vfs, &m_usmapData, &m_globalData);
    Crawler crawler;
    crawler.crawl(parser);
  }

  void SatisfactoryDataProvider::test() {
    AssetParser parser(&m_vfs, &m_usmapData, &m_globalData);
    parser.parseAsset("FactoryGame/Content/FactoryGame/Buildable/Factory/ResourceSink/Build_ResourceSink.uasset",
                      [](std::shared_ptr<AssetFileParser> &file) {
                        file->loadExports();
                        file->exportAssets();
                      });
    DBG_PRINT_MISSING();
  }

  void SatisfactoryDataProvider::findAllParsableFiles(const std::filesystem::path &folder) {
    if (!std::filesystem::exists(folder) || !std::filesystem::is_directory(folder)) {
      ProviderLog::warn("Pfad {} ist ungültig.", folder.string());
      return;
    }
    auto t = ProviderLog::time("Loaded Files in {}");


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
            ProviderLog::warn("UTOC with no UCAS: {}", path.string());
          }
        } else if (ext == ".usmap") {
          m_UsmapParser = std::make_unique<UsmapReader>(path);
        }
      }
    }
  }


  void SatisfactoryDataProvider::setECS(SC::ChaosRegistry *reg) {
    Exporter::setECS(reg);
  }
}
