//
// Created by oleub on 06.02.26.
//

#include "loc_manager.hpp"

#include <algorithm>

#include "AssetParser.hpp"
#include "logger.hpp"
#include "vfs.hpp"

namespace SAA {
  LocManager::LocManager(AssetParser *parser) : m_parser(parser) {
    auto start = std::chrono::high_resolution_clock::now();

    auto *vfs = parser->getVfs();
    for (auto &entry: vfs->entries()) {
      if (entry.second->path.ends_with(".locmeta")) {
        auto &parser = m_metaParsers.emplace_back(this, entry.second);
        parser.parse();
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[LocManager]", "Found {} Files in: {} ms", m_metaParsers.size() ,us.count() / 1000.0);
  }

  void LocManager::setCulture(std::string_view langCode) {
    if (!m_registeredCultures.contains(std::string(langCode))) {
      Logger::logError("[LOC]", "Could not find Culture: {}", langCode);
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

    if (cultureIndex == loadedIndex) {
      return;
    }
    auto *vfs = m_parser->getVfs();
    auto culture = m_cultureList.at(cultureIndex);

    auto start = std::chrono::high_resolution_clock::now();
    entries.clear();
    m_resParsersNative.clear();
    m_cultureRes.clear();
    m_cultureRes.reserve(m_metaParsers.size());
    m_resParsersNative.reserve(m_metaParsers.size());
    for (const auto &locMeta: m_metaParsers) {
      auto nativeResPath = locMeta.getNativeRes();
      auto vfsEntryNative = vfs->readVfs(nativeResPath);
      if (vfsEntryNative) {
        auto *res = &m_resParsersNative.emplace_back(this, vfsEntryNative);
        m_jobQueue.emplace(res);
      } else {
        Logger::logError("[LOC]", "Failed to load locRes: {}", nativeResPath);
      }

      auto [isNative, resPath] = locMeta.getRes(culture);
      if (!isNative && !resPath.empty()) {
        auto vfsEntry = vfs->readVfs(resPath);
        if (vfsEntry) {
          auto *res = &m_cultureRes.emplace_back(this, vfsEntry);
          m_jobQueue.emplace(res);
        } else {
          Logger::logError("[LOC]", "Failed to load locRes: {}", resPath);
        }
      }
    }

    processCultures();
    for (auto &res: m_cultureRes) {
      mergeCultures(res.getMap());
    }
    for (auto &res: m_resParsersNative) {
      mergeCultures(res.getMap());
    }

    loadedIndex = cultureIndex;
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[LOC]", "Loaded Culture: {} in {} ms", culture, us.count() / 1000.0);
  }

  void LocManager::processCultures(uint32_t threadCount) {
    if (threadCount == 0) {
      threadCount = std::max(1u, (std::thread::hardware_concurrency() - 1));
    }
    for (uint32_t i = 0; i < threadCount; i++) {
      m_ThreadPool.emplace_back(&LocManager::workerThread, this);
    }

    for (uint32_t i = 0; i < threadCount; ++i) {
      if (m_ThreadPool[i].joinable()) {
        m_ThreadPool[i].join();
      }
    }
    m_ThreadPool.clear();
  }

  void LocManager::workerThread() {
    while (true) {
      std::unique_lock lock(m_QueueMutex);
      if (m_jobQueue.empty()) {
        return;
      }
      auto parser = m_jobQueue.front();
      m_jobQueue.pop();
      lock.unlock();
      parser->parse();
    }
  }

  void LocManager::mergeCultures(std::unordered_map<FTextKey, std::unordered_map<FTextKey, FEntry>> &src) {
    for (auto &[nsName, nsKeys]: src) {
      auto dst = entries.find(nsName);
      if (dst == entries.end()) {
        entries[nsName] = std::move(nsKeys);
      } else {
        dst->second.merge(entries[nsName]);
      }
    }
  }
} // namespace SAP
