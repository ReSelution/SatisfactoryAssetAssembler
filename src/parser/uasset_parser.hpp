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

        [[nodiscard]] const FZenPackageSummary &getSummary() const;

        [[nodiscard]] std::span<std::string_view> getNameMap() const;

        [[nodiscard]] std::span<const FBulkDataMapEntry> bulkDataMap() const { return m_bulkDataMap; };

        FByteBulkData *parseFByteBulkData(ByteCursor &cursor) override;

        Object *resolveIndex(int32_t index) override;

        Object *resolveSoftObj(std::string_view assetName) override;

        void loadExports() const override;

        void exportAssets() const override;

        void reset() override;

    protected:
        void parse() override;

    private:
        // ---- high level pipeline ----
        void parseSummary(GameFile * file);

        void parseVersioning(GameFile * file);

        void parseCellOffsets(GameFile * file);

        void parseNameMap(GameFile * file);

        void resolveStoreEntry();

        void parseBulkDataMap(GameFile * file);

        void parseImports(GameFile * file);

        void parseExports(GameFile * file);

        void parseExportBundles(GameFile * file);

        void loadImportedPackages();

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

        using exportDenseMap = ankerl::unordered_dense::pmr::map<uint64_t, uint32_t, SC::IdentityHash>;
        exportDenseMap m_exportHashToIndex;
        using nameDenseMap = ankerl::unordered_dense::pmr::map<std::string_view, uint32_t, SC::IdentityHash>;
        nameDenseMap m_nameToIndex;
        std::span<const FExportBundleEntry> m_exportBundleEntries;

        // ---- imported assets ----
        std::span<Object *> m_importObjects;
        std::span<Object *> m_exportObjects;
    };
}
