//
// Created by oleub on 07.12.25.
//

#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "sc/memory/monotonic_resource.hpp"

#include "io/packages/f_export_map_entry.hpp"
#include "io_global_data.hpp"
#include "version.hpp"

namespace SAA {
  struct FByteBulkData;
  class Reader;
  struct FUsmapMappingData;
  struct GlobalData;
  class UnifiedVfsEntry;
  class AssetParser;
  class Object;
  class ByteCursor;
  class GameFile;

  // What parseFByteBulkData() does with a payload that lies in a separate file (.ubulk).
  enum class EBulkLoad : uint8_t {
    // Decompressed into the storage of the file right away, FByteBulkData::bulkCursor points at it.
    Storage,
    // Only located. The caller fetches it with FByteBulkData::readInto() into a buffer of its own, or not at all.
    Deferred,
  };

  enum class ParserState : uint8_t {
    NONE,
    PARSING,
    LOADED,
    RESETTING,
    // parse() threw. safeParse() throws for every caller until safeReset() brings the parser back to NONE.
    FAILED,
  };


  class AssetFileParser {
  public:
    enum class AssetType { UASSET, UMAP, UNKNOWN };

    AssetFileParser(AssetParser *assetParser, std::string_view path, AssetType assetType = AssetType::UNKNOWN) :
        m_assetParser(assetParser), m_type(assetType), m_path(path) {
      auto lastDot = m_path.find_last_of('.');

      if (lastDot != std::string::npos) {
        m_path = m_path.substr(0, lastDot);
      }
    }

    virtual ~AssetFileParser();

    [[nodiscard]] ParserState state() const { return m_state.load(std::memory_order_acquire); }

    void safeParse();

    void safeReset();


    virtual Object *resolveIndex(int32_t index [[maybe_unused]]) { return nullptr; };
    virtual Object *resolveSoftObj(std::string_view assetName [[maybe_unused]]) { return nullptr; }


    [[nodiscard]] std::string_view nameFromMap(uint32_t index) const {
      if (index < m_nameMap.size()) [[likely]] {
        return m_nameMap[index];
      }
      return "None";
    }

    [[nodiscard]] std::string_view getMappedName(FMappedName name) const {
      if (name.isGlobal()) {
        const auto &map = globalData()->globalNameMap;
        return name.fromNameMap(map, m_storage.get());
      }
      return name.fromNameMap(m_nameMap, m_storage.get());
    }

    std::span<std::string_view> nameMap() const { return m_nameMap; }


    virtual Object *resolveExportObject(const FExportMapEntry &entry [[maybe_unused]],
                                        const FPackageObjectIndex &index [[maybe_unused]]) {
      return {};
    };

    virtual Object *resolvePackageExport(const uint64_t &publicExportHash [[maybe_unused]]) { return nullptr; };

    [[nodiscard]] const FUsmapMappingData *mappings() const;

    [[nodiscard]] EUE5Version getUE5Version() const;

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

    virtual FByteBulkData *parseFByteBulkData(ByteCursor &cursor, EBulkLoad load = EBulkLoad::Storage) = 0;

    //    [[nodiscard]] ByteCursor getAsset();
    [[nodiscard]] GameFile *getAssetFile();

    [[nodiscard]] ByteCursor getAsset(size_t offset, size_t size);

    [[nodiscard]] ByteCursor getBulk();

    [[nodiscard]] GameFile *getBulkFile();

    virtual std::span<Object *> exportMap() = 0;

    virtual void loadExports() const = 0;

    virtual void exportAssets() const = 0;

    [[nodiscard]] std::pmr::vector<std::shared_ptr<AssetFileParser>> *importetParser() const {
      return m_importedParsers;
    }

    [[nodiscard]] sc::MonotonicResource *storage() { return m_storage.get(); }

  protected:
    virtual void parse() { throw std::runtime_error("AssetFileParser::parse() called"); };
    std::span<std::string_view> m_nameMap;
    AssetParser *m_assetParser = nullptr;
    // The imported packages that were asked for so far, in the order they were asked for.
    std::pmr::vector<std::shared_ptr<AssetFileParser>> *m_importedParsers{};
    // Kommt in safeParse() aus dem sc::ArenaPool und geht in reset() dorthin zurueck, vor dem ersten safeParse()
    // also nullptr.
    // mutable: const-Methoden wie getMappedName() allokieren Namen daraus.
    mutable std::unique_ptr<sc::MonotonicResource> m_storage;

    virtual void reset();

  private:
    Reader *m_reader = nullptr;

    // ---- raw data ----
    // Both files are opened on first use, which can come from several threads at once: the objects of a package
    // are parsed in parallel and each may ask for its bulk data.
    const UnifiedVfsEntry *m_assetEntry{};
    std::atomic<GameFile *> m_asset = nullptr;
    const UnifiedVfsEntry *m_bulkEntry{};
    std::atomic<GameFile *> m_bulk = nullptr;
    std::mutex m_fileLock;

    AssetType m_type;
    ;
    std::string_view m_path;

    std::atomic<ParserState> m_state = ParserState::NONE;
    std::mutex m_mutex;
    std::condition_variable m_cv;
  };
} // namespace SAA
