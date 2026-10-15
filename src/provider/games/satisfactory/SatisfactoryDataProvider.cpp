#include "games/satisfactory/SatisfactoryDataProvider.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>

#include <memory>

#include <future>
#include <ranges>

#include "AssetParser.hpp"
#include "Decompressor.hpp"
#include "IOStoreReader.hpp"
#include "PAKReader.hpp"
#include "PropertyTracker.hpp"
#include "UsmapReader.hpp"
#include "crawler.hpp"
#include "io_global_data.hpp"
#include "runtime_options.hpp"

#include "exporter.hpp"
#include "sc/localizations/magpie_string.hpp"
#include "sc/logger/logger.hpp"
#include "stats.hpp"
#include "sc/stats/core.hpp"
#include "sc/threading/threading.hpp"

using ProviderLog = sc::Logger<"SAA", "Provider">;

namespace SAA {
  SatisfactoryDataProvider::SatisfactoryDataProvider(const std::filesystem::path &baseInstallPath) {
    ProviderLog::info("Searching in {}", baseInstallPath.string());

    findAllParsableFiles(baseInstallPath);

    OodleLoader::loadOodle(baseInstallPath);
  }

  bool SatisfactoryDataProvider::load() {
    auto t = ProviderLog::time("Loaded in {}");
    if (runtimeOptions().serial) {
      ProviderLog::info("Running serial: sequential load.");

      // 1. Basis-Parser direkt ausführen
      m_UsmapParser->parse(m_usmapData);
      m_globalDataParser->parse(m_globalData);

      // 2. Without other threads the readers mount straight into the Vfs.
      for (const auto &val: m_Parsers | std::views::values) {
        auto *currentParser = val.get();

        currentParser->load();
        currentParser->mountVFS(&m_vfs);
      }
    } else {
      std::vector<std::future<void>> parsableFutures;
      parsableFutures.reserve(m_Parsers.size() + 2);
      parsableFutures.emplace_back(sc::threading::enqueue([&]() { m_UsmapParser->parse(m_usmapData); }));
      parsableFutures.emplace_back(sc::threading::enqueue([&]() { m_globalDataParser->parse(m_globalData); }));

      std::vector<Vfs> childVFS;
      const size_t slots = sc::threading::threadSlots();
      childVFS.reserve(slots);
      for (size_t i = 0; i < slots; i++) {
        childVFS.emplace_back(m_vfs.makeChild());
      }

      for (const auto &val: m_Parsers | std::views::values) {
        auto *currentParser = val.get();
        parsableFutures.emplace_back(sc::threading::enqueue([&, currentParser]() {
          auto &child = childVFS[sc::threading::threadId()];
          currentParser->load();
          currentParser->mountVFS(&child);
          m_vfs.mergeChild(child);
        }));
      }

      // All tasks have to be done before the first get() may throw, they hold references to the locals above.
      for (auto &f: parsableFutures) {
        f.wait();
      }
      for (auto &f: parsableFutures) {
        f.get();
      }
    }
    // sc::threading::detach([&]() {
    //   m_vfs.dumpToText("output/files.txt");;
    // });

    ProviderLog::info("VFS erfolgreich erstellt. {} Dateien gemountet.", m_vfs.entries().size());
    return true;
  }

  void SatisfactoryDataProvider::crawl() {
    AssetParser parser(&m_vfs, &m_usmapData, &m_globalData);
    sc::stats::resetAll();
    DecompressedThroughput::reset();
    CompressedThroughput::reset();
    parser.loadRes();
    Crawler crawler;
    crawler.crawl(parser);

    sc::Magpie::get().dumpToFile("output/magpie.json");
  }

  void SatisfactoryDataProvider::test() {
    AssetParser parser(&m_vfs, &m_usmapData, &m_globalData);


    ProviderLog::info("Testing with {} files", m_vfs.entries().size());
    auto val = m_vfs.entries().values();
    sc::stats::resetAll();
    DecompressedThroughput::reset();
    CompressedThroughput::reset();
    // A file that cannot be parsed is reported and skipped, like in Crawler::createFileQueue().
    std::atomic<size_t> files  = 0;
    std::atomic<size_t> failed = 0;
    sc::threading::detachBatch(
        val,
        [&parser, &files, &failed](auto entry) {
          if (entry.second->isPkg) {
            files.fetch_add(1, std::memory_order_relaxed);
            try {
              parser.parseAsset(entry.second->packagePath, [](std::shared_ptr<AssetFileParser> &&file) {
                file->loadExports();
                // file->exportAssets();
              });
            } catch (const std::exception &e) {
              failed.fetch_add(1, std::memory_order_relaxed);
              ProviderLog::err("{}: {}", entry.second->packagePath, e.what());
            }
          }
        },
        nullptr);
    sc::threading::waitUntilFinished();
    ProviderLog::info("Parsed {} Files", files.load());
    if (failed > 0) {
      ProviderLog::err("{} of {} Files could not be parsed", failed.load(), files.load());
    }
    ProviderLog::stats<DecompressedThroughput, CompressedThroughput>("Crawled");
    sc::stats::reportAll<ProviderLog>();
    // DBG_PRINT_MISSING();
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
        auto path     = entry.path();
        auto ext      = path.extension().string();
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


  void SatisfactoryDataProvider::setECS(sc::ecs::Registry *reg) { Exporter::setECS(reg); }
} // namespace SAA
