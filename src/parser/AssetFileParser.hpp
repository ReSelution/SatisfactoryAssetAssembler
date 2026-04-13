//
// Created by oleub on 07.12.25.
//

#pragma once
#include <cstdint>
#include <condition_variable>

#include "memory/chaos_arena_pool.hpp"
#include "io/packages/f_byte_bulk_data.hpp"
#include "io/packages/f_export_map_entry.hpp"
#include "io/packages/f_object_export.hpp"

namespace SAA {
  class Reader;
  struct FUsmapMappingData;
  struct GlobalData;
  class UnifiedVfsEntry;
  class AssetParser;
  class Object;

  enum class ParserState : uint8_t {
    NONE,
    PARSING,
    LOADED,
    RESETTING,
  };


  class AssetFileParser {
  public:
    enum class AssetType { UASSET, UMAP, UNKNOWN };

    AssetFileParser(AssetParser *assetParser, std::string_view path,
                    AssetType assetType = AssetType::UNKNOWN) : m_assetParser(assetParser), m_type(assetType),
                                                                m_path(path) {
      auto lastDot = m_path.find_last_of('.');

      if (lastDot != std::string::npos) {
        m_path = m_path.substr(0, lastDot);
      }
    }

    virtual ~AssetFileParser();

    [[nodiscard]] ParserState state() const { return m_state.load(std::memory_order_acquire); }

    void safeParse();
    void safeReset();

    [[nodiscard]] const AssetType type();

    virtual Object *resolveIndex(int32_t index) { return nullptr; };
    virtual Object *resolveSoftObj(std::string_view assetName) { return nullptr; }

    virtual std::string_view nameFromMap(uint32_t index) = 0;

    virtual std::span<std::string_view> nameMap() = 0;

    virtual Object *resolveExportObject(const FExportMapEntry &entry, const FPackageObjectIndex &index) { return {}; };

    virtual Object *resolvePackageExport(const uint64_t &publicExportHash) { return nullptr; };

    [[nodiscard]] const FUsmapMappingData *mappings() const;

    [[nodiscard]] const GlobalData *globalData() const;

    [[nodiscard]] AssetParser *assetParser() const;

    [[nodiscard]] std::string_view path() const { return m_path; }

    void setAssetData(const UnifiedVfsEntry *assetEntry) {
      if (m_state.load(std::memory_order_acquire) == ParserState::NONE) {
        m_assetEntry = assetEntry;
      }
    };

    void setBulkData(const UnifiedVfsEntry *bulkEntry) {
      if (m_state.load(std::memory_order_acquire) == ParserState::NONE) {
        m_bulkEntry = bulkEntry;
      }
    };

    void setReader(Reader *reader);

    virtual FByteBulkData parseFByteBulkData(ByteCursor &cursor) = 0;

    [[nodiscard]] ByteCursor getAsset();

    [[nodiscard]] ByteCursor getBulk();

    virtual std::span<Object *> exportMap() = 0;

    virtual void loadExports() const = 0;

    virtual void exportAssets() const = 0;

    std::pmr::vector<std::shared_ptr<AssetFileParser> > *importetParser() const { return m_importedParsers; }
    [[nodiscard]] SC::ChaosBumpArena *storage() const { return m_storage.get(); }

  protected:
    virtual void parse() {
      throw std::runtime_error("AssetFileParser::parse() called");
    };
    AssetParser *m_assetParser;
    std::pmr::vector<std::shared_ptr<AssetFileParser> > *m_importedParsers;
    std::unique_ptr<SC::ChaosBumpArena> m_storage;

    virtual void reset();

  private:
    Reader *m_reader = nullptr;

    // ---- raw data ----
    const UnifiedVfsEntry *m_assetEntry;
    std::span<uint8_t> m_asset{};
    const UnifiedVfsEntry *m_bulkEntry;
    std::span<uint8_t> m_bulk{};

    AssetType m_type;
    ;
    std::string_view m_path;

    std::atomic<ParserState> m_state = ParserState::NONE;
    std::mutex m_mutex;
    std::condition_variable m_cv;
  };
}
