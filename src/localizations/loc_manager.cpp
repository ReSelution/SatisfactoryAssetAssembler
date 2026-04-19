//
// Created by oleub on 06.02.26.
//

#include "loc_manager.hpp"

#include <algorithm>

#include "AssetParser.hpp"
#include "logger/chaos_logger.hpp"
#include "vfs.hpp"
#include "threading/chaos_threading.hpp"

LOG_ALIAS(LocMngLog, "SAA", "LOC")
DEFINE_CHAOS_STAT(LocResThr, "LocRes Throughput",  SC::ChaosThroughput<SC::MetricUnits>);

namespace SAA {
  LocManager::LocManager(AssetParser *parser) : m_parser(parser) {
    auto size = m_metaParsers.size();
    auto t = LocMngLog::time("Found {1} in {0}", size);

    std::vector<std::future<void> > futures;
    auto *vfs = parser->getVfs();
    for (auto &entry: vfs->entries()) {
      if (entry.second->path.ends_with(".locmeta")) {
        auto &parser = m_metaParsers.emplace_back(this, entry.second);
        parser.parse();
      }
    }

    size = m_metaParsers.size();
  }

  void LocManager::setCulture(std::string_view langCode) {
    if (!m_registeredCultures.contains(std::string(langCode))) {
      LocMngLog::err("[LOC]", "Could not find Culture: {}", langCode);
      return;
    }
    auto index = 0;
    for (auto &entry: m_cultureList) {
      if (entry == langCode) {
        break;
      }
      index++;
    }
    cultureIndex = index;
  }

  void LocManager::registerCulture(std::string_view culture) {
    auto [it, inserted] = m_registeredCultures.insert(std::string(culture));
    if (inserted) {
      m_cultureList.push_back(*it);
      if (*it == DEFAULT_CULTURE) {
        cultureIndex = m_registeredCultures.size() - 1;
      }
    }
  }

  void LocManager::load() {
    setCulture("de");
    if (cultureIndex == loadedIndex) {
      return;
    }
    auto *vfs = m_parser->getVfs();
    auto culture = m_cultureList.at(cultureIndex);


    std::vector<LocResParser> parsers;
    parsers.reserve(m_metaParsers.size() * 2);

    auto loadLoc = [&parsers, vfs](const std::string_view path, bool isNative = false) {
      if (path.empty()) [[unlikely]] return;
      if (auto vfsEntry = vfs->readVfs(path)) {
        parsers.emplace_back(vfsEntry, isNative);
      } else [[unlikely]] {
        LocMngLog::err("[LOC]", "Failed to load locRes: {}", path);
      }
    };

    for (const auto& locMeta : m_metaParsers) {
      loadLoc(locMeta.getNativeRes(), true);
      if (auto [isNative, resPath] = locMeta.getRes(culture); !isNative) {
        loadLoc(resPath);
      }
    }
#ifndef NDEBUG
    for (auto *parser: parsers) {
      parser.parse();
    }
#else


      SC::Magpie::get()->clear();
      auto t = SC::make_timer([magpie = SC::Magpie::get()](SC::TimeResult res) {
        LocMngLog::info("Parsed {} Strings in {:.2f}{}", magpie->size(),  res.value, res.suffix);
      });
      SC::ChaosThreading::detacheBatch<SC::ChaosThreading::Priority::High>(std::move(parsers), [](int id, auto parser) {
        parser.parse();
      }, [timer = std::move(t)](int id) mutable {
        timer.stop();
      });


#endif
    loadedIndex = cultureIndex;

  }

} // namespace SAA
