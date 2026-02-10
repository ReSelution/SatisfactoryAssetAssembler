#pragma once

#include "AssetFileParser.hpp"
#include "../util/byte_cursor.hpp"
#include "version.hpp"
#include "core/name.hpp"
#include "io/f_io_container_header.hpp"
#include "io/packages/f_bulk_data_map_entry.hpp"
#include "io/packages/f_byte_bulk_data.hpp"
#include "io/packages/f_export_bundle_entry.hpp"
#include "io/packages/f_zen_package_cell_offsets.hpp"
#include "io/packages/f_zen_package_summary.hpp"
#include "ue/object/struct.hpp"

namespace SAP {
  class IOVfsEntry;
  class Object;
  class AssetParser;

  class UAssetParser final : public AssetFileParser {
  public:
    UAssetParser(AssetParser *assetParser, const IOVfsEntry* vfsEntry);
    ~UAssetParser() override = default;

    [[nodiscard]] std::string_view nameFromMap(uint32_t index) override;
    std::string_view getMappedName(FMappedName name);

    [[nodiscard]] const FZenPackageSummary &getSummary() const;
    [[nodiscard]] const std::vector<std::string_view> &getNameMap() const;

    [[nodiscard]] std::span<const FBulkDataMapEntry> bulkDataMap() const { return m_bulkDataMap; };

    FByteBulkData parseFByteBulkData(ByteCursor &cursor) override;

    std::weak_ptr<Object> resolveIndex(int32_t index) override;

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
    void processExports();

    void processExportEntry(const FExportBundleEntry &entry);
    std::weak_ptr<Object> resolveObject(const FExportMapEntry &entry, const FPackageObjectIndex &index);
    std::weak_ptr<Object> resolveExportObject
    (const FExportMapEntry &entry, const FPackageObjectIndex &index) override;
    std::weak_ptr<Object> resolveScriptImport(const FExportMapEntry &entry, const FPackageObjectIndex &index);
    std::weak_ptr<Object> resolvePackageImport(const FExportMapEntry &entry, const FPackageObjectIndex &index) const;
    std::weak_ptr<Object> resolvePackageExport(const uint64_t &publicExportHash) override;
    std::weak_ptr<Object> constructObject(const FExportMapEntry &entry, const std::shared_ptr<Struct> &ustruct);
    std::vector<std::shared_ptr<Object> > &exportMap() override;

    const FIoContainerHeader *m_containerHeader{nullptr};

    // ---- summary / version ----
    FZenPackageSummary m_summary{};
    FPackageFileVersion m_version{};
    FZenPackageCellOffsets m_cellOffsets{};

    // ---- name & store ----
    HeapStorage<> m_stringStorage;
    std::vector<std::string_view> m_nameMap;
    FFilePackageStoreEntry m_storeEntry{};

    // ---- data maps ----
    std::span<const FBulkDataMapEntry> m_bulkDataMap;
    std::span<const FPackageObjectIndex> m_importMap;
    std::span<const uint64_t> m_importedPublicExportHashes;
    std::span<const FExportMapEntry> m_exportMap;
    std::unordered_map<uint64_t, uint32_t> m_exportHashToIndex;
    std::span<const FExportBundleEntry> m_exportBundleEntries;

    // ---- imported assets ----
    std::vector<std::shared_ptr<Object> > m_exportObjectsKeepalive;

    std::vector<std::weak_ptr<Object> > m_exportObjects;
  };
} // namespace SAP
