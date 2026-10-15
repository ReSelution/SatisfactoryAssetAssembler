//
// Created by oleub on 07.12.25.
//

#pragma once
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

#include "loc_manager.hpp"
#include "sc/memory/arena_pool.hpp"
#include "shared_cache.hpp"


namespace SAA {
  struct GlobalData;
  class Crawler;
  struct FPackageId;
  class Reader;
  class AssetFileParser;
} // namespace SAA

namespace SAA {
  struct AssetParseResult;
  class Vfs;
  struct FUsmapMappingData;

  class AssetParser {
  public:
    explicit AssetParser(Vfs *vfs, const FUsmapMappingData *mappings, const GlobalData *globalData);
    ~AssetParser() = default;

    void parseAsset(std::string_view path, const std::function<void(std::shared_ptr<AssetFileParser> &&)> &onParsed);

    std::shared_ptr<AssetFileParser> loadPackage(std::string_view path);
    std::shared_ptr<AssetFileParser> loadPackage(FPackageId packageId);
    Vfs *getVfs() const { return m_vfs; };

    void loadRes();

  private:
    friend class AssetFileParser;
    [[nodiscard]] const FUsmapMappingData *mappings() const { return m_mappings; }
    [[nodiscard]] const GlobalData *globalData() const { return m_globalData; }

    std::shared_ptr<AssetFileParser> createParser(const UnifiedVfsEntry *vfsEntry);
    Vfs *m_vfs;
    const FUsmapMappingData *m_mappings;
    const GlobalData *m_globalData;

    LocManager m_localizationManager;

    // The arenas of the AssetFileParsers. Declared before m_parserCache so that it outlives the cached parsers.
    sc::ArenaPool m_arenaPool;

    ShardedCache<uint64_t, AssetFileParser> m_parserCache;
  };
} // namespace SAA
