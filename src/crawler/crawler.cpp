//
// Created by oleub on 30.01.26.
//

#include "crawler.hpp"

#include <algorithm>

#include "AssetParser.hpp"
#include "stats.hpp"
#include "vfs.hpp"
#include "ue/asset_object.hpp"

#include "logger/chaos_logger.hpp"
#include "puplic/arena/registery.hpp"
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
    std::ranges::move(child.m_buildable, std::back_inserter(m_buildable));
    std::ranges::move(child.m_recipes, std::back_inserter(m_recipes));
    std::ranges::move(child.m_items, std::back_inserter(m_items));
    std::ranges::move(child.m_schematics, std::back_inserter(m_schematics));
    std::ranges::move(child.m_resources, std::back_inserter(m_resources));
    m_buildable.shrink_to_fit();
    m_recipes.shrink_to_fit();
    m_items.shrink_to_fit();
    m_schematics.shrink_to_fit();
    m_resources.shrink_to_fit();
  }

  auto Crawler::createFileQueue(AssetParser &parser) {
    Vfs *vfs = parser.getVfs();
    std::vector<std::string_view> batch;
    batch.reserve(vfs->entries().size());
    for (const auto &entry: vfs->entries()) {
      if (shouldAdd(entry.second->path)) {
        batch.emplace_back(entry.second->path);
      }
    }
    return SC::ChaosThreading::enqueueBatch(SC::ChaosThreading::Priority::Normal,
                                            batch.begin(), batch.end(), [&parser, this](int id, std::string_view path) {
                                              parser.parseAsset(path, [&child = m_childs[id]](std::shared_ptr<AssetFileParser> &file) {
                                                child.onParsed(file);
                                              });
                                            });
  }


  void Crawler::crawl(AssetParser &parser) {
    for (int i = 0; i < SC::ChaosThreading::getNumThreads(); ++i) {
      m_childs.emplace_back(getChild());
    }
    size_t files;
    auto t = CrawlerLog::time("Parsed {1} Files in {0}", files);
    SC::ChaosSlabPool::init(SC::ChaosThreading::getNumThreads() * 4, 8 * 1024);
    Registry::init(2048 * 1024);
    DecompressedThroughput::reset();
    CompressedThroughput::reset();
    auto futures = createFileQueue(parser);
    files = futures.size();
    for (auto &fut: futures) {
      fut.wait();
    }
    for (auto & m_child : m_childs) {
      mergeWithChild(m_child);
    }
    t.stop();
    DecompressedThroughput::stop();
    CompressedThroughput::stop();
    DBG_PRINT_MISSING();
    CrawlerLog::info("Found {} Buildings", m_buildable.size());
    CrawlerLog::info("Found {} Recipes", m_recipes.size());
    CrawlerLog::info("Found {} Items", m_items.size());
    CrawlerLog::info("Found {} Schematics", m_schematics.size());
    CrawlerLog::info("Found {} Resources", m_resources.size());

    CrawlerLog::stats<DecompressedThroughput,CompressedThroughput>("Crawled");
    SC::ChaosGlobal::report_all();

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
} // namespace SAP
