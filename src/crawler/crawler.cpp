//
// Created by oleub on 30.01.26.
//

#include "crawler.hpp"

#include <atomic>
#include <exception>

#include "AssetParser.hpp"
#include "asset_object.hpp"
#include "satisfactory/building/fg_building_descriptor.hpp"
#include "satisfactory/fg_recipe.hpp"
#include "satisfactory/fg_resource_descriptor.hpp"
#include "satisfactory/fg_schematic.hpp"
#include "satisfactory/items/fg_item_descriptor.hpp"
#include "satisfactory/items/fg_item_descriptor_biomass.hpp"
#include "runtime_options.hpp"
#include "stats.hpp"
#include "vfs.hpp"

#include "sc/logger/logger.hpp"
#include "sc/memory/arena_pool.hpp"
#include "sc/stats/core.hpp"
#include "sc/threading/threading.hpp"

using CrawlerLog = sc::Logger<"SAA", "CRAWLER">;

namespace SAA {
  Crawler::Crawler() {
    m_buildable.reserve(100);
    m_recipes.reserve(100);
    m_items.reserve(100);
    m_schematics.reserve(100);
    m_resources.reserve(10);
  }

  Crawler Crawler::getChild() { return {}; }

  void Crawler::mergeWithChild(Crawler &child) {
    auto merge = [](auto &source, auto &target) {
      target.insert(target.end(), std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()));
      source.clear();
    };

    merge(child.m_buildable, m_buildable);
    merge(child.m_recipes, m_recipes);
    merge(child.m_items, m_items);
    merge(child.m_schematics, m_schematics);
    merge(child.m_resources, m_resources);
  }

  auto Crawler::createFileQueue(AssetParser &parser, size_t &files, size_t &failed) {
    Vfs *vfs = parser.getVfs();
    files    = 0;

    // A file that cannot be parsed is reported and skipped, the remaining files are still crawled.
    std::atomic<size_t> failures = 0;
    const auto parseFile         = [&parser, &failures](std::string_view path, Crawler &sink) {
      try {
        parser.parseAsset(path, [&sink](std::shared_ptr<AssetFileParser> &&file) { sink.onParsed(std::move(file)); });
      } catch (const std::exception &e) {
        failures.fetch_add(1, std::memory_order_relaxed);
        CrawlerLog::err("{}: {}", path, e.what());
      }
    };

    if (runtimeOptions().serial) {
      for (const auto &entry: vfs->entries()) {
        if (shouldAdd(entry.second)) {
          files++;
          parseFile(entry.second->packagePath, *this);
        }
      }
      failed = failures.load(std::memory_order_relaxed);
      return;
    }

    std::vector<std::string_view> batch;
    batch.reserve(vfs->entries().size());
    for (const auto &entry: vfs->entries()) {
      if (shouldAdd(entry.second)) {
        batch.emplace_back(entry.second->packagePath);
      }
    }
    files    = batch.size();
    auto fut = sc::threading::enqueueBatch(std::move(batch), [&parseFile, this](std::string_view path) {
      parseFile(path, m_childs[sc::threading::threadId()]);
    });
    // get() instead of wait(): anything that is not a std::exception still ends the batch and has to reach main().
    fut.get();
    failed = failures.load(std::memory_order_relaxed);
  }


  void Crawler::crawl(AssetParser &parser) {
    const size_t slots = sc::threading::threadSlots();
    for (size_t i = 0; i < slots; ++i) {
      m_childs.emplace_back(getChild());
    }
    size_t files   = 0;
    size_t failed  = 0;
    size_t threads = runtimeOptions().serial ? 1 : slots - 1;

    auto t = CrawlerLog::time("Parsed {1} Files in {0} on {2} Threads", files, threads);
    createFileQueue(parser, files, failed);
    for (auto &m_child: m_childs) {
      mergeWithChild(m_child);
    }
    m_childs.clear();
    t.stop();
    DBG_PRINT_MISSING();
    if (failed > 0) {
      CrawlerLog::err("{} of {} Files could not be parsed", failed, files);
    }
    CrawlerLog::info("Found {} Buildings", m_buildable.size());
    CrawlerLog::info("Found {} Recipes", m_recipes.size());
    CrawlerLog::info("Found {} Items", m_items.size());
    CrawlerLog::info("Found {} Schematics", m_schematics.size());
    CrawlerLog::info("Found {} Resources", m_resources.size());
    CrawlerLog::stats<DecompressedThroughput, CompressedThroughput>("Crawled");
    sc::stats::reportAll<CrawlerLog>();
  }

  void Crawler::onParsed(std::shared_ptr<AssetFileParser> &&parser) { hitTestAsset(std::move(parser)); }

  bool Crawler::hitTestAsset(std::shared_ptr<AssetFileParser> &&parser) {
    for (auto &obj: parser->exportMap()) {
      if (!obj) {
        continue;
      }
      auto hash = obj->getClassHash();
      switch (hash) {
        case FGRecipe::StaticHASH:
          obj->safeExport();
          m_recipes.emplace_back(std::move(parser));
          return true;

        case FGSchematic::StaticHASH:
          obj->safeExport();
          m_schematics.emplace_back(std::move(parser));
          return true;

        case FGItemDescriptor::StaticHASH:
        case FGItemDescriptorBiomass::StaticHASH:
          obj->safeExport();
          m_items.emplace_back(std::move(parser));
          return true;

        case FGResourceDescriptor::StaticHASH:
          obj->safeExport();
          m_resources.emplace_back(std::move(parser));
          return true;

        case FGBuildingDescriptor::StaticHASH:
          obj->safeExport();
          m_buildable.emplace_back(std::move(parser));
          return true;

        default:
          break;
      }
    }
    return false;
  }


  bool Crawler::shouldAdd(const UnifiedVfsEntry *entry) { return entry->isPkg; }
} // namespace SAA
