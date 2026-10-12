//
// Created by oleub on 06.02.26.
//

#include "loc_manager.hpp"

#include <algorithm>
#include <atomic>
#include <memory>

#include "AssetParser.hpp"
#include "runtime_options.hpp"
#include "sc/logger/logger.hpp"
#include "sc/stats/stats.hpp"
#include "sc/stats/throughput.hpp"
#include "sc/stats/timer.hpp"
#include "sc/threading/threading.hpp"
#include "vfs.hpp"

using LocMngLog = sc::Logger<"SAA", "LOC">;

namespace SAA {
  LocManager::LocManager(AssetParser *parser) : m_parser(parser) {
    auto size = m_metaParsers.size();
    auto t = LocMngLog::time("Found {1} in {0}", size);

    auto *vfs = parser->getVfs();
    for (auto &entry: vfs->entries()) {
      if (entry.second->pathStr.ends_with(".locmeta")) {
        auto &parser = m_metaParsers.emplace_back(this, entry.second);
        parser.parse();
      }
    }

    size = m_metaParsers.size();

    if (const auto &culture = runtimeOptions().culture; !culture.empty()) {
      setCulture(culture);
    }
  }

  void LocManager::setCulture(std::string_view langCode) {
    if (!m_registeredCultures.contains(std::string(langCode))) {
      LocMngLog::err("Could not find Culture: {}", langCode);
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
    }
  }

  void LocManager::load() {
    if (m_loaded && cultureIndex == loadedIndex) {
      return;
    }
    auto *vfs = m_parser->getVfs();
    const std::string_view culture = cultureIndex ? m_cultureList.at(*cultureIndex) : std::string_view{};


    std::vector<LocResParser> parsers;
    parsers.reserve(m_metaParsers.size() * 2);

    auto loadLoc = [&parsers, vfs](const std::string_view path, bool isNative = false) {
      if (path.empty()) [[unlikely]] return;
      if (auto vfsEntry = vfs->readVfs(path)) {
        parsers.emplace_back(vfsEntry, isNative);
      } else [[unlikely]] {
        LocMngLog::err("Failed to load locRes: {}", path);
      }
    };

    for (const auto &locMeta: m_metaParsers) {
      loadLoc(locMeta.getNativeRes(), true);
      if (culture.empty()) {
        continue;
      }
      if (auto [isNative, resPath] = locMeta.getRes(culture); !isNative) {
        loadLoc(resPath);
      }
    }
    if (runtimeOptions().serial) {
      for (auto &parser: parsers) {
        parser.parse();
      }
    } else {
      sc::Magpie::get()->clear();
      // Counted here instead of magpie->size(): the crawl is already running and inserts its own texts into the same
      // map, so the size depends on when the batch finishes. Every entry of every file counts, the same key from the
      // native and the translated file counts twice.
      auto inserted = std::make_shared<std::atomic<size_t>>(0);
      sc::stats::Timer t([inserted](sc::stats::TimeResult res) {
        LocMngLog::info("Parsed {} Entries in {:.2f}{}", inserted->load(std::memory_order_relaxed), res.value,
                        res.suffix);
      });
      sc::threading::detachBatch<sc::threading::Priority::High>(
          std::move(parsers),
          [inserted](LocResParser parser) { inserted->fetch_add(parser.parse(), std::memory_order_relaxed); },
          [timer = std::move(t)]() mutable {
            timer.stop();
            LocMngLog::stats<sc::MagpieInsert, sc::MagpieMEM, sc::threading::PoolThroughput>("Magpie");
          });
    }
    loadedIndex = cultureIndex;
    m_loaded    = true;
  }
} // namespace SAA
