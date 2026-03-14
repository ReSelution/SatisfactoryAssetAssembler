#pragma once

#include "byte_cursor.hpp"
#include "AssetFileParser.hpp"
#include "core/name.hpp"
#include "io/f_io_container_header.hpp"
#include "io/packages/f_bulk_data_map_entry.hpp"
#include "io/packages/f_byte_bulk_data.hpp"
#include "io/packages/f_export_bundle_entry.hpp"
#include "io/packages/f_zen_package_cell_offsets.hpp"
#include "io/packages/f_zen_package_summary.hpp"
#include "ue/object/struct.hpp"
#include "version.hpp"

namespace SAA {
  class IOVfsEntry;
  class Object;
  class AssetParser;

  class UAssetParser final : public AssetFileParser {
  public:
    UAssetParser(AssetParser *assetParser, const IOVfsEntry *vfsEntry);
    ~UAssetParser() override = default;

    [[nodiscard]] std::string_view nameFromMap(uint32_t index) override;
    std::span<std::string_view> nameMap() override;
    std::string_view getMappedName(FMappedName name);

    [[nodiscard]] const FZenPackageSummary &getSummary() const;
    [[nodiscard]] std::span<std::string_view> getNameMap() const;

    [[nodiscard]] std::span<const FBulkDataMapEntry> bulkDataMap() const { return m_bulkDataMap; };

    FByteBulkData parseFByteBulkData(ByteCursor &cursor) override;

    Object* resolveIndex(int32_t index) override;
    Object* resolveSoftObj(std::string_view assetName) override;

    void loadExports() const override;
    void exportAssets() const override;
    void reset() override;

  protected:
    void parse() override;

  private:
    // ---- high level pipeline ----
    void parseSummary(ByteCursor &cursor);
    void parseVersioning(ByteCursor &cursor);
    void parseCellOffsets(ByteCursor &cursor);
    void parseNameMap(ByteCursor &cursor);
    void resolveStoreEntry();
    void parseBulkDataMap(ByteCursor &cursor);
    void parseImports(ByteCursor &cursor);
    void parseExports(ByteCursor &cursor);
    void parseExportBundles(ByteCursor &cursor);
    void loadImportedPackages();
    void processImportes();
    void processExports();

    void processExportEntry(const FExportBundleEntry &entry);
    Object* resolveObject(const FExportMapEntry &entry, const FPackageObjectIndex &index, bool copy);
    Object* resolveExportObject(const FExportMapEntry &entry, const FPackageObjectIndex &index) override;
    Object* resolveScriptImport(const FExportMapEntry &entry, const FPackageObjectIndex &index, bool copy);
    Object* resolvePackageImport(const FExportMapEntry &entry, const FPackageObjectIndex &index, bool copy) ;
    Object* resolvePackageExport(const uint64_t &publicExportHash) override;
    Object* constructObject(const FExportMapEntry &entry, Struct* ustruct, bool copy);
    std::span<Object*> exportMap() override;

    const FIoContainerHeader *m_containerHeader{nullptr};

    // ---- summary / version ----
    FZenPackageSummary m_summary{};
    FPackageFileVersion m_version{};
    FZenPackageCellOffsets m_cellOffsets{};

    // ---- name & store ----
    std::span<std::string_view> m_nameMap;
    FFilePackageStoreEntry m_storeEntry{};

    // ---- data maps ----
    std::span<const FBulkDataMapEntry> m_bulkDataMap;
    std::span<const FPackageObjectIndex> m_importMap;
    std::span<const uint64_t> m_importedPublicExportHashes;
    std::span<const FExportMapEntry> m_exportMap;
    std::pmr::unordered_map<uint64_t, uint32_t> m_exportHashToIndex;
    std::span<const FExportBundleEntry> m_exportBundleEntries;
    std::pmr::unordered_map<std::string_view, uint32_t> m_nameToIndex;

    // ---- imported assets ----
    std::span<Object *> m_importObjects;
    std::span<Object *> m_exportObjects;
  };
} // namespace SAP
