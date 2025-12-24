#pragma once

#include "AssetFileParser.hpp"
#include "version.hpp"
#include "core/name.hpp"
#include "io/f_io_container_header.hpp"
#include "io/packages/f_bulk_data_map_entry.hpp"
#include "io/packages/f_export_bundle_entry.hpp"
#include "io/packages/f_zen_package_cell_offsets.hpp"
#include "io/packages/f_zen_package_summary.hpp"

namespace SAP {


  class AssetParser;

  class UAssetParser final : public AssetFileParser {
  public:
    UAssetParser(AssetParser* assetParser,std::string path,  const FIoContainerHeader* containerHeader);
    ~UAssetParser() override = default;

    AssetParseResult parse(
        const std::vector<uint8_t>& asset,
        const std::vector<uint8_t>& bulk) override;

    [[nodiscard]] std::string nameFromMap(const FMappedName& mappedName) const;

    [[nodiscard]] const std::vector<uint8_t>& getAsset() const;
    [[nodiscard]] const std::vector<uint8_t>& getBulk() const;

    [[nodiscard]] const FZenPackageSummary& getSummary() const;
    [[nodiscard]] const std::vector<std::string>& getNameMap() const;

  private:
    // ---- high level pipeline ----
    void parseSummary();
    void parseVersioning();
    void parseCellOffsets();
    void parseNameMap();
    void resolveStoreEntry();
    void parseBulkDataMap();
    void parseImports();
    void parseExports();
    void parseExportBundles();
    void loadImportedPackages();
    void processExports();

    // ---- export handling ----
    void processExportEntry(const FExportBundleEntry& entry);

    // ---- external ----
    const FIoContainerHeader* m_containerHeader{nullptr};

    // ---- raw data ----
    const std::vector<uint8_t>* m_asset{nullptr};
    const std::vector<uint8_t>* m_bulk{nullptr};
    size_t m_offset{0};

    // ---- summary / version ----
    FZenPackageSummary m_summary{};
    FPackageFileVersion m_version{};
    FZenPackageCellOffsets m_cellOffsets{};

    // ---- name & store ----
    std::vector<std::string> m_nameMap;
    FFilePackageStoreEntry m_storeEntry{};

    // ---- data maps ----
    std::vector<FBulkDataMapEntry> m_bulkDataMap;
    std::vector<FPackageObjectIndex> m_importMap;
    std::vector<uint64_t> m_importedPublicExportHashes;
    std::vector<FExportMapEntry> m_exportMap;
    std::vector<FExportBundleEntry> m_exportBundleEntries;

    // ---- imported assets ----
    std::vector<std::unique_ptr<AssetFileParser>> m_importedParsers;
  };

} // namespace SAP
