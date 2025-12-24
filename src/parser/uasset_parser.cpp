#include "uasset_parser.hpp"

#include "AssetParser.hpp"
#include "../assets/ue/object_factory.hpp"
#include "functions.hpp"
#include "logger.hpp"
#include "core/game.hpp"
#include "io/packages/f_zen_package_versioning_info.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAP {
  UAssetParser::UAssetParser
  (
    AssetParser *assetParser,
    std::string path, const FIoContainerHeader *containerHeader)
    : AssetFileParser(assetParser, std::move(path), AssetType::UASSET),
      m_containerHeader(containerHeader) {}

  AssetParseResult UAssetParser::parse
  (
    const std::vector<uint8_t> &asset,
    const std::vector<uint8_t> &bulk) {
    m_asset = &asset;
    m_bulk = &bulk;
    m_offset = 0;

    parseSummary();
    parseVersioning();
    parseCellOffsets();
    parseNameMap();
    resolveStoreEntry();
    parseBulkDataMap();
    parseImports();
    parseExports();
    parseExportBundles();
    loadImportedPackages();
    processExports();

    return {};
  }

  std::string UAssetParser::nameFromMap(const FMappedName &mappedName) const {
    if (mappedName.isGlobal()) {
      return mappedName.fromNameMap(globalData()->globalNameMap);
    } else {
      return mappedName.fromNameMap(m_nameMap);
    }
  }

  void UAssetParser::parseSummary() {
    m_summary = readStruct<FZenPackageSummary>(*m_asset, m_offset);

    if (m_summary.Name.isGlobal()) { throw std::runtime_error("Global Name Map not implemented"); }

    Logger::logError("[UASSET]", "Summary parsed, offset={}", m_offset);
  }

  void UAssetParser::parseVersioning() {
    m_version = mappings()->pkgVersion;

    if (m_summary.bHasVersioningInfo != 0) {
      const auto info =
          readStruct<FZenPackageVersioningInfo>(*m_asset, m_offset);
      m_version = info.PackageVersion;
    }
  }

  void UAssetParser::parseCellOffsets() {
    if (m_summary.bHasVersioningInfo == 0 &&
        m_version.FileVersionUE5 >= EUnrealEngineObjectUE5Version::VERSE_CELLS) {
      m_cellOffsets =
          readStruct<FZenPackageCellOffsets>(*m_asset, m_offset);
    } else {
      m_cellOffsets.CellImportMapOffset = m_summary.ExportBundleEntriesOffset;
      m_cellOffsets.CellExportMapOffset = m_summary.ExportBundleEntriesOffset;
    }
  }

  void UAssetParser::parseNameMap() {
    m_nameMap = loadNameBatch(*m_asset, m_offset);

    Logger::logError("[UASSET]", "NameMap size={}", m_nameMap.size());
  }

  void UAssetParser::resolveStoreEntry() {
    const std::string name =
        m_summary.Name.fromNameMap(m_nameMap);

    const auto [packageId] = FPackageId::fromName(name);

    for (size_t i = 0; i < m_containerHeader->packageIds.size(); ++i) {
      if (m_containerHeader->packageIds[i].Id == packageId) {
        m_storeEntry = m_containerHeader->storeEntries[i];
        return;
      }
    }

    Logger::logError("[UASSET]", "StoreEntry not found for {}", name);
  }

  bool operator<(uint32_t lhs, EUnrealEngineObjectUE5Version rhs) {
    return static_cast<uint32_t>(lhs) < static_cast<uint32_t>(rhs);
  };

  void UAssetParser::parseBulkDataMap() {
    if (m_version.FileVersionUE5 < EUnrealEngineObjectUE5Version::DATA_RESOURCES) { return; }

    if (m_version.FileVersionUE5 >= EGame::UE5_4) {
      const uint64_t padding =
          readPrimitive<uint64_t>(*m_asset, m_offset);
      m_offset += padding;
    }

    const uint64_t mapSize =
        readPrimitive<uint64_t>(*m_asset, m_offset);

    m_bulkDataMap = readArray<FBulkDataMapEntry>
        (
         *m_asset,
         m_offset,
         mapSize / sizeof(FBulkDataMapEntry));
  }

  void UAssetParser::parseImports() {
    m_offset = m_summary.ImportedPublicExportHashesOffset;

    m_importedPublicExportHashes =
        readArray<uint64_t>
        (
         *m_asset,
         m_offset,
         (m_summary.ImportMapOffset - m_offset) / sizeof(uint64_t));

    m_offset = m_summary.ImportMapOffset;
    m_importMap =
        readArray<FPackageObjectIndex>
        (
         *m_asset,
         m_offset,
         m_summary.importCount());
  }

  void UAssetParser::parseExports() {
    m_offset = m_summary.ExportMapOffset;
    m_exportMap =
        readArray<FExportMapEntry>
        (
         *m_asset,
         m_offset,
         m_summary.exportCount());
  }

  void UAssetParser::parseExportBundles() {
    m_offset = m_cellOffsets.CellImportMapOffset;

    m_exportBundleEntries =
        readArray<FExportBundleEntry>
        (
         *m_asset,
         m_offset,
         m_summary.exportCount() * 2);
  }

  void UAssetParser::loadImportedPackages() {
    m_importedParsers.reserve(m_storeEntry.importedPackage.size());

    for (const auto &pkgId: m_storeEntry.importedPackage) {
      Logger::logError("[UASSET]", "Imported package {}", pkgId.Id);
      m_importedParsers.emplace_back
          (
           m_assetParser->loadPackage(pkgId));
    }
  }

  void UAssetParser::processExports() { for (const auto &entry: m_exportBundleEntries) { processExportEntry(entry); } }

  void UAssetParser::processExportEntry(const FExportBundleEntry &entry) {
    if (entry.CommandType != EExportCommandType::Serialize) { return; }

    const auto &exportEntry =
        m_exportMap[entry.LocalExportIndex];

    size_t cursor =
        m_summary.HeaderSize + exportEntry.CookedSerialOffset;

    Logger::logError
        (
         "[ENTRY]",
         "Export {}",
         exportEntry.ObjectName.fromNameMap(m_nameMap));

    if (!exportEntry.ClassIndex.IsScriptImport()) { return; }

    const auto &scriptEntry =
        globalData()->ScriptObjectEntriesMap.at(exportEntry.ClassIndex);

    auto object = ObjectFactory::Create(nameFromMap(scriptEntry.ObjectName));

    if (!object) { return; }
    object->setObjName(exportEntry.ObjectName.fromNameMap(m_nameMap));
    Logger::logInfo("[ENTRY]", "Class {}", object->getClassName());
    object->parse(cursor, this);
  }

  const std::vector<uint8_t> &UAssetParser::getAsset() const {
    assert(m_asset && "Asset buffer not set");
    return *m_asset;
  }

  const std::vector<uint8_t> &UAssetParser::getBulk() const {
    assert(m_bulk && "Bulk buffer not set");
    return *m_bulk;
  }

  const FZenPackageSummary &UAssetParser::getSummary() const { return m_summary; }

  const std::vector<std::string> &UAssetParser::getNameMap() const { return m_nameMap; }
}
