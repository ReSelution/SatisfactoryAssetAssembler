//
// Created by oleub on 30.01.26.
//

#include "crawler.hpp"

#include <algorithm>

#include "AssetParser.hpp"
#include "stats.hpp"
#include "vfs.hpp"
#include "asset_object.hpp"

#include "logger/chaos_logger.hpp"
#include "threading/chaos_threading.hpp"

LOG_ALIAS(CrawlerLog, "SAA", "CRAWLER")

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
      target.insert(target.end(),
                    std::make_move_iterator(source.begin()),
                    std::make_move_iterator(source.end()));
      source.clear();
    };

    merge(child.m_buildable, m_buildable);
    merge(child.m_recipes, m_recipes);
    merge(child.m_items, m_items);
    merge(child.m_schematics, m_schematics);
    merge(child.m_resources, m_resources);
  }

  auto Crawler::createFileQueue(AssetParser &parser, size_t &files) {
    Vfs *vfs = parser.getVfs();
    files = 0;
#ifndef NDEBUG
    for (const auto &entry: vfs->entries()) {
      if (shouldAdd(entry.second->path)) {
        files++;
        parser.parseAsset(entry.second->path, [&](std::shared_ptr<AssetFileParser> &file) {
          onParsed(file);
        });
      }
    }

#else
    std::vector<std::string_view> batch;
    batch.reserve(vfs->entries().size());
    for (const auto &entry: vfs->entries()) {
      if (shouldAdd(entry.second->path)) {
        batch.emplace_back(entry.second->path);
      }
    }
    files = batch.size();
    auto fut =  SC::ChaosThreading::enqueueBatch(
      std::move(batch), [&parser, this](int id, std::string_view path) {
        parser.parseAsset(
          path, [this, id](std::shared_ptr<AssetFileParser> &file) {
            this->m_childs[id].onParsed(file);
          });
      });
    fut.wait();
#endif
  }


  void Crawler::crawl(AssetParser &parser) {
    for (int i = 0; i < SC::ChaosThreading::getNumThreads(); ++i) {
      m_childs.emplace_back(getChild());
    }
    size_t files;
    auto threads = SC::ChaosThreading::getNumThreads();

    auto t = CrawlerLog::time("Parsed {1} Files in {0} on {2} Threads", files, threads);
    SC::ChaosArenaPool::init(SC::ChaosThreading::getNumThreads() * 16, 32 * 1024);
    createFileQueue(parser, files);
    for (auto &m_child: m_childs) {
      mergeWithChild(m_child);
    }
    m_childs.clear();
    t.stop();
    DBG_PRINT_MISSING();
    CrawlerLog::info("Found {} Buildings", m_buildable.size());
    CrawlerLog::info("Found {} Recipes", m_recipes.size());
    CrawlerLog::info("Found {} Items", m_items.size());
    CrawlerLog::info("Found {} Schematics", m_schematics.size());
    CrawlerLog::info("Found {} Resources", m_resources.size());
    CHAOS_STATS_LOG(CrawlerLog, "Crawled", DecompressedThroughput, CompressedThroughput)
    CHAOS_REPORT()
  }

  void Crawler::onParsed(std::shared_ptr<AssetFileParser> &parser) {
    hitTestAsset(parser);
  }

  bool Crawler::hitTestAsset(std::shared_ptr<AssetFileParser> &parser) {
    for (auto &obj: parser->exportMap()) {
      if (!obj) { continue; }
      auto hash = static_cast<HIT_TEST_CLASS_HASH>(obj->getClassHash());
      switch (hash) {
        case HIT_TEST_CLASS_HASH::FGRecipe:
          obj->safeExport();
          m_recipes.emplace_back(parser);
          return true;

        case HIT_TEST_CLASS_HASH::FGSchematic:
          obj->safeExport();
          m_schematics.emplace_back(parser);
          return true;

        case HIT_TEST_CLASS_HASH::FGItemDescriptor:
        case HIT_TEST_CLASS_HASH::FGItemDescriptorBiomass:
          obj->safeExport();
          m_items.emplace_back(parser);
          return true;

        case HIT_TEST_CLASS_HASH::FGResourceDescriptor:
          obj->safeExport();
          m_resources.emplace_back(parser);
          return true;

        case HIT_TEST_CLASS_HASH::FGBuildingDescriptor:
          obj->safeExport();
          m_buildable.emplace_back(parser);
          return true;

        default:
          break;
      }
    }
    return false;
  }


  bool Crawler::shouldAdd(std::string_view path) {
    return std::any_of(filterList.begin(), filterList.end(),
                       [path](std::string_view filter) { return path.find(filter) != std::string_view::npos; });
  }
}
