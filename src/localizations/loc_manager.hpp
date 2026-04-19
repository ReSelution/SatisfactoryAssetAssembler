//
// Created by oleub on 06.02.26.
//

#pragma once
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "loc_meta_parser.hpp"
#include "loc_res_parser.hpp"
#include "localizations/magpie_translator.hpp"

namespace std {
  class thread;
}

namespace SAA {
  class AssetParser;
  constexpr std::string_view DEFAULT_CULTURE = "en-US";

  class LocManager {
  public:
    LocManager(AssetParser *parser);

    void setCulture(std::string_view langCode);

    void registerCulture(std::string_view culture);

    void load();

  private:
    void mergeCultures(SC::magpieMAP &culture);

    AssetParser *m_parser = nullptr;

    size_t cultureIndex = 0;
    size_t loadedIndex = -1;

    std::set<std::string> m_registeredCultures;
    std::vector<std::string_view> m_cultureList;

    std::vector<LocMetaParser> m_metaParsers;

    // std::unordered_map<FTextKey, std::unordered_map<FTextKey, FEntry>> entries;
  };
}
