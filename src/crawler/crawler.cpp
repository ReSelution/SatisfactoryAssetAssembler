//
// Created by oleub on 30.01.26.
//

#include "crawler.hpp"

#include <algorithm>

#include "AssetParser.hpp"
#include "vfs.hpp"

#include "ue/asset_object.hpp"
#include "PropertyTracker.hpp"

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

  void Crawler::crawl(AssetParser &parser) {
    createFileQueue(parser);
    parser.processQueue(this);
    DBG_PRINT_MISSING();
    Logger::logInfo("[CRAWLER]", "Found {} Buildings", m_buildable.size());
    Logger::logInfo("[CRAWLER]", "Found {} Recipes", m_recipes.size());
    Logger::logInfo("[CRAWLER]", "Found {} Items", m_items.size());
    Logger::logInfo("[CRAWLER]", "Found {} Schematics", m_schematics.size());
    Logger::logInfo("[CRAWLER]", "Found {} Resources", m_resources.size());
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

  void Crawler::createFileQueue(AssetParser &parser) {
    Vfs *vfs = parser.getVfs();

    for (const auto &entry: vfs->entries()) {
      if (shouldAdd(entry.second->path)) {
        parser.queueFile(entry.second->path);
      }
    }
  }

  bool Crawler::shouldAdd(std::string_view path) {
    return std::any_of(filterList.begin(), filterList.end(),
                       [path](std::string_view filter) { return path.find(filter) != std::string_view::npos; });
  }
} // namespace SAP
