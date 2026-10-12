//
// Created by oleub on 06.02.26.
//

#pragma once
#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "loc_meta_parser.hpp"
#include "loc_res_parser.hpp"
#include "sc/localizations/magpie_translator.hpp"

namespace SAA {
  class AssetParser;

  class LocManager {
  public:
    LocManager(AssetParser *parser);

    void setCulture(std::string_view langCode);

    void registerCulture(std::string_view culture);

    void load();

  private:
    AssetParser *m_parser = nullptr;

    // Empty until setCulture() picks one: every localization target is then loaded in its native culture.
    std::optional<size_t> cultureIndex;
    std::optional<size_t> loadedIndex;
    bool m_loaded = false;

    std::set<std::string> m_registeredCultures;
    std::vector<std::string_view> m_cultureList;

    std::vector<LocMetaParser> m_metaParsers;

    // std::unordered_map<FTextKey, std::unordered_map<FTextKey, FEntry>> entries;
  };
}
