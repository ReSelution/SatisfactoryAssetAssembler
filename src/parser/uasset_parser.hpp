#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>

#include "AssetFileParser.hpp"
#include "byte_cursor.hpp"
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

    [[nodiscard]] const FZenPackageSummary &getSummary() const;

    [[nodiscard]] std::span<std::string_view> getNameMap() const;

    [[nodiscard]] std::span<const FBulkDataMapEntry> bulkDataMap() const { return m_bulkDataMap; };

    FByteBulkData *parseFByteBulkData(ByteCursor &cursor, EBulkLoad load = EBulkLoad::Storage) override;

    Object *resolveIndex(int32_t index) override;

    Object *resolveSoftObj(std::string_view assetName) override;

    void loadExports() const override;

    void exportAssets() const override;

    void reset() override;

  protected:
    void parse() override;

  private:
    // ---- high level pipeline ----
    void parseSummary(GameFile *file);

    void parseVersioning(GameFile *file);

    void parseCellOffsets(GameFile *file);

    void parseNameMap(GameFile *file);

    void resolveStoreEntry();

    void parseBulkDataMap(GameFile *file);

    void parseImports(GameFile *file);

    void parseExports(GameFile *file);

    void parseExportBundles(GameFile *file);

    void loadImportedPackages();

    // The parser of an imported package, by its index in the store entry. Made on the first call, which can come
    // from several threads at once. nullptr if the package is not in the VFS.
    [[nodiscard]] AssetFileParser *importedPackage(size_t index);

    void processExports();

    void processExportEntry(const FExportBundleEntry &entry);

    Object *resolveObject(const FExportMapEntry &entry, const FPackageObjectIndex &index, bool copy);

    Object *resolveExportObject(const FExportMapEntry &entry, const FPackageObjectIndex &index) override;

    Object *resolveScriptImport(const FExportMapEntry &entry, const FPackageObjectIndex &index, bool copy);

    Object *resolvePackageImport(const FExportMapEntry &entry, const FPackageObjectIndex &index, bool copy);

    Object *resolvePackageExport(const uint64_t &publicExportHash) override;

    Object *constructObject(const FExportMapEntry &entry, Struct *ustruct, bool copy);

    std::span<Object *> exportMap() override;

    const FIoContainerHeader *m_containerHeader{nullptr};

    // ---- summary / version ----
    FZenPackageSummary m_summary{};
    FPackageFileVersion m_version{};
    FZenPackageCellOffsets m_cellOffsets{};

    // ---- name & store ----

    FFilePackageStoreEntry m_storeEntry{};

    // ---- data maps ----
    std::span<const FBulkDataMapEntry> m_bulkDataMap;
    std::span<const FPackageObjectIndex> m_importMap;
    std::span<const uint64_t> m_importedPublicExportHashes;
    std::span<const FExportMapEntry> m_exportMap;

    using exportDenseMap = ankerl::unordered_dense::pmr::map<uint64_t, uint32_t, sc::IdentityHash>;
    exportDenseMap m_exportHashToIndex;
    using nameDenseMap = ankerl::unordered_dense::pmr::map<std::string_view, uint32_t, sc::IdentityHash>;
    nameDenseMap m_nameToIndex;
    std::span<const FExportBundleEntry> m_exportBundleEntries;

    // One slot per imported package: nullptr until importedPackage() was asked for it, unknownPackage() if there
    // is no such package. The parsers themselves are held by m_importedParsers.
    std::span<std::atomic<AssetFileParser *>> m_importSlots;
    std::mutex m_importLock;
    [[nodiscard]] static AssetFileParser *unknownPackage() {
      return reinterpret_cast<AssetFileParser *>(uintptr_t{1});
    }

    // ---- imported assets ----
    std::span<Object *> m_importObjects;
    std::span<Object *> m_exportObjects;
  };
} // namespace SAA
