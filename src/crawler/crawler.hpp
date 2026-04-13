//
// Created by oleub on 30.01.26.
//

#pragma once
#include <memory>

#include "AssetFileParser.hpp"

#include "hash/hash.hpp"


namespace SAA {
  class Crawler {
      enum class HIT_TEST_CLASS_HASH : uint64_t {

          // --- REZEPTE & FORSCHUNG ---
          FGRecipe                 = "FGRecipe"_h,
          FGSchematic              = "FGSchematic"_h,

          // --- ITEMS & DESKRIPTOREN ---
          FGItemDescriptor         = "FGItemDescriptor"_h,
          FGItemDescriptorBiomass  = "FGItemDescriptorBiomass"_h,
          FGResourceDescriptor     = "FGResourceDescriptor"_h,
          FGBuildingDescriptor     = "FGBuildingDescriptor"_h,
      };

  public:
    Crawler();
    ~Crawler() {
      m_childs.clear();

      m_buildable.clear();
      m_recipes.clear();
      m_items.clear();
      m_resources.clear();
    }
    static Crawler getChild();
    void mergeWithChild(Crawler &child);
    void crawl(AssetParser &parser);

    void onParsed(std::shared_ptr<AssetFileParser> &parser);

    static constexpr std::array<std::string_view, 1> filterList = {
        ".uasset",
    };

  private:
    bool hitTestAsset(std::shared_ptr<AssetFileParser> &parser);
    auto createFileQueue(AssetParser &parser);
    bool shouldAdd(std::string_view path);

    // Keeping the Parser Alive for Cache
    std::vector<std::shared_ptr<AssetFileParser>> m_buildable;
    std::vector<std::shared_ptr<AssetFileParser>> m_recipes;
    std::vector<std::shared_ptr<AssetFileParser>> m_items;
    std::vector<std::shared_ptr<AssetFileParser>> m_schematics;
    std::vector<std::shared_ptr<AssetFileParser>> m_resources;

    std::vector<Crawler> m_childs;

  };
} 
