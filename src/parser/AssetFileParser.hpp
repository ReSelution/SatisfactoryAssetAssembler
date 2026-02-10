//
// Created by oleub on 07.12.25.
//

#pragma once
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>

#include "io/packages/f_byte_bulk_data.hpp"
#include "io/packages/f_export_map_entry.hpp"
#include "io/packages/f_file_summary.hpp"
#include "io/packages/f_object_export.hpp"
#include "io/packages/f_object_import.hpp"

namespace SAP {
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
  };

  struct AssetParseResult {
    std::filesystem::path FilePath;
    bool bSuccess = false;

    std::vector<std::string> NameMap;
    std::vector<FObjectImport> Imports;
    std::vector<FObjectExport> Exports;
    FFileSummary Summary{};
  };

  class AssetFileParser {
  public:
    enum class AssetType {
      UASSET,
      UMAP,
      UNKNOWN
    };

    AssetFileParser
    (AssetParser *assetParser, std::string_view path, AssetType assetType = AssetType::UNKNOWN) : m_assetParser
      (assetParser), m_type(assetType), m_path(std::move(path)) {
      auto lastDot = m_path.find_last_of('.');

      if (lastDot != std::string::npos) { m_path = m_path.substr(0, lastDot); }
    }

    virtual void reset();
    virtual ~AssetFileParser() = default;
    [[nodiscard]] ParserState state() const { return m_state.load(std::memory_order_acquire); }
    void safeParse();
    [[nodiscard]] const AssetType type();

    virtual std::weak_ptr<Object> resolveIndex(int32_t index) { return {}; };
    virtual std::string_view nameFromMap(uint32_t index) = 0;

    virtual std::weak_ptr<Object> resolveExportObject
    (const FExportMapEntry &entry, const FPackageObjectIndex &index) { return {}; };

    virtual std::weak_ptr<Object> resolvePackageExport
    (const uint64_t &publicExportHash) { return std::weak_ptr<Object>(); };

    [[nodiscard]] const FUsmapMappingData *mappings() const;
    [[nodiscard]] const GlobalData *globalData() const;
    std::string_view path() { return m_path; }

    void setAssetData(const UnifiedVfsEntry *assetEntry) {
      if (m_state.load(std::memory_order_acquire) == ParserState::NONE) { m_assetEntry = assetEntry; }
    };

    void setBulkData(const UnifiedVfsEntry *bulkEntry) {
      if (m_state.load(std::memory_order_acquire) == ParserState::NONE) { m_bulkEntry = bulkEntry; }
    };

    void setReader(Reader *reader);
    virtual FByteBulkData parseFByteBulkData(ByteCursor &cursor) = 0;
    [[nodiscard]] ByteCursor getAsset();
    [[nodiscard]] ByteCursor getBulk();

    virtual std::vector<std::shared_ptr<Object> > &exportMap() = 0;
    virtual void loadExports() const = 0;
    virtual void exportAssets() const = 0;

    std::vector<std::shared_ptr<AssetFileParser> > &importetParser() { return m_importedParsers; }

  protected:
    virtual void parse() = 0;
    AssetParser *m_assetParser;
    std::vector<std::shared_ptr<AssetFileParser> > m_importedParsers;

  private:
    Reader *m_reader = nullptr;
    // ---- raw data ----
    const UnifiedVfsEntry *m_assetEntry;
    std::vector<uint8_t> m_asset;
    const UnifiedVfsEntry *m_bulkEntry;
    std::vector<uint8_t> m_bulk;

    AssetType m_type;;
    std::string_view m_path;

    std::atomic<ParserState> m_state = ParserState::NONE;
    std::mutex m_mutex;
    std::condition_variable m_cv;
  };

  inline void AssetFileParser::reset() {
    m_importedParsers.clear();
    m_asset.clear();
    m_bulk.clear();
    m_state.store(ParserState::NONE, std::memory_order_release);
  }

  inline void AssetFileParser::safeParse() {
    if (m_state.load(std::memory_order_acquire) == ParserState::LOADED) { return; }
    ParserState expected = ParserState::NONE;
    if (m_state.compare_exchange_strong(expected, ParserState::PARSING, std::memory_order_acq_rel)) {
      parse();
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state.store(ParserState::LOADED, std::memory_order_release);
      }
      m_cv.notify_all();
      return;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) != ParserState::PARSING; });
  }
}
