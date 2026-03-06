//
// Created by oleub on 30.01.26.
//

#pragma once
#include <memory>

#include "AssetFileParser.hpp"

#include "core/hash.hpp"


namespace SAA {
  class Crawler {
    enum class HIT_TEST_CLASS_HASH : uint32_t {

      // --- REZEPTE & FORSCHUNG ---
      FGRecipe = fnv1a("FGRecipe"),
      FGSchematic = fnv1a("FGSchematic"),

      // --- ITEMS & DESKRIPTOREN ---
      FGItemDescriptor = fnv1a("FGItemDescriptor"),
      FGItemDescriptorBiomass = fnv1a("FGItemDescriptorBiomass"),
      FGResourceDescriptor = fnv1a("FGResourceDescriptor"),
      FGBuildingDescriptor = fnv1a("FGBuildingDescriptor"), // Item-Form von Gebäuden

    };

  public:
    Crawler();
    static Crawler getChild();
    void mergeWithChild(Crawler &child);
    void crawl(AssetParser &parser);

    void onParsed(std::shared_ptr<AssetFileParser> &parser);

    static constexpr std::array<std::string_view, 1> filterList = {
        ".uasset",
    };

  private:
    bool hitTestAsset(std::shared_ptr<AssetFileParser> &parser);
    void createFileQueue(AssetParser &parser);
    bool shouldAdd(std::string_view path);

    // Keeping the Parser Alive for Cache
    std::vector<std::shared_ptr<AssetFileParser>> m_buildable;
    std::vector<std::shared_ptr<AssetFileParser>> m_recipes;
    std::vector<std::shared_ptr<AssetFileParser>> m_items;
    std::vector<std::shared_ptr<AssetFileParser>> m_schematics;
    std::vector<std::shared_ptr<AssetFileParser>> m_resources;

  };
} // namespace SAP
