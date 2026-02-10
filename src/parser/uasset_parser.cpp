#include "uasset_parser.hpp"

#include "AssetParser.hpp"
#include "../assets/ue/object_factory.hpp"
#include "IOStoreReader.hpp"
#include "io_global_data.hpp"
#include "logger.hpp"
#include "vfs_entry.hpp"

#include "io/packages/f_zen_package_versioning_info.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAP {
  UAssetParser::UAssetParser
  (AssetParser *assetParser, const IOVfsEntry* vfsEntry)
    : AssetFileParser(assetParser, vfsEntry->path, AssetType::UASSET) {
    auto reader = vfsEntry->owningReader;
    m_containerHeader = reader->containerHeader();
  }

  void UAssetParser::parse() {
    auto cursor = getAsset();

    parseSummary(cursor);
    parseVersioning(cursor);
    parseCellOffsets(cursor);
    parseNameMap(cursor);
    resolveStoreEntry();
    parseBulkDataMap(cursor);
    parseImports(cursor);
    parseExports(cursor);
    parseExportBundles(cursor);
    loadImportedPackages();
    processExports();
  }

  std::string_view UAssetParser::nameFromMap(uint32_t index) { return m_nameMap[index]; }

  std::string_view UAssetParser::getMappedName(FMappedName name) {
    if (name.isGlobal()) { return name.fromNameMap(globalData()->globalNameMap, m_stringStorage); }
    return name.fromNameMap(m_nameMap, m_stringStorage);
  }

  FByteBulkData UAssetParser::parseFByteBulkData(ByteCursor &cursor) {
    FByteBulkData data{};
    auto dataIndex = cursor.read<int32_t>();
    if (dataIndex >= m_bulkDataMap.size()) { return data; }
    auto &metaData = m_bulkDataMap[dataIndex];
    data.flags = static_cast<EBulkDataFlags>(metaData.Flags);
    if (HasFlag(data.flags, EBulkDataFlags::BULKDATA_ForceInlinePayload)) {
      data.bulkCursor = cursor.subspan(cursor.tell(), metaData.SerialSize);
      cursor += metaData.SerialSize;
    } else if (HasFlag(data.flags, EBulkDataFlags::BULKDATA_PayloadInSeperateFile)) {
      auto bulkCursor = getBulk();
      data.bulkCursor = bulkCursor.subspan(metaData.SerialOffset, metaData.SerialSize);
    }

    return data;
  }

  std::weak_ptr<Object> UAssetParser::resolveIndex(int32_t index) {
    if (index == 0) { return {}; }
    if (index < 0 && (-index - 1) < m_importMap.size()) { return resolveObject({}, m_importMap[(-index - 1)]); }
    if (index > 0 && index - 1 < m_exportMap.size()) {
      return resolveExportObject({}, m_exportMap[index - 1].ClassIndex);
    }
    return {};
  }

  void UAssetParser::parseSummary(ByteCursor &cursor) { m_summary = cursor.read<FZenPackageSummary>(); }

  void UAssetParser::parseVersioning(ByteCursor &cursor) {
    m_version = mappings()->pkgVersion;

    if (m_summary.bHasVersioningInfo != 0) {
      const auto info = cursor.read<FZenPackageVersioningInfo>();
      m_version = info.PackageVersion;
    }
  }

  void UAssetParser::parseCellOffsets(ByteCursor &cursor) {
    if (m_summary.bHasVersioningInfo == 0 &&
        m_version.FileVersionUE5 >= EUnrealEngineObjectUE5Version::VERSE_CELLS) {
      m_cellOffsets = cursor.read<FZenPackageCellOffsets>();
    } else {
      m_cellOffsets.CellImportMapOffset = m_summary.ExportBundleEntriesOffset;
      m_cellOffsets.CellExportMapOffset = m_summary.ExportBundleEntriesOffset;
    }
  }

  void UAssetParser::parseNameMap(ByteCursor &cursor) { m_nameMap = cursor.loadNameBatch(m_stringStorage); }

  void UAssetParser::resolveStoreEntry() {
    std::string_view name = getMappedName(m_summary.Name);

    const auto packageId = FPackageId::fromName(name);
    auto entry = m_containerHeader->storeEntriesById.find(packageId);
    if (entry != m_containerHeader->storeEntriesById.end()) {
      m_storeEntry = entry->second;
      return;
    }
    Logger::logError("[UASSET]", "StoreEntry not found for {}", name);
  }

  void UAssetParser::parseBulkDataMap(ByteCursor &cursor) {
    if (m_version.FileVersionUE5 < EUnrealEngineObjectUE5Version::DATA_RESOURCES) { return; }

    if (m_version.FileVersionUE5 >= EUE5Version::UE5_4) {
      auto padding = cursor.read<uint64_t>();
      cursor += padding;
    }

    const auto mapSize =
        cursor.read<uint64_t>();

    m_bulkDataMap = cursor.readArray<FBulkDataMapEntry>
        (
         mapSize / sizeof(FBulkDataMapEntry));
  }

  void UAssetParser::parseImports(ByteCursor &cursor) {
    cursor = m_summary.ImportedPublicExportHashesOffset;

    m_importedPublicExportHashes =
        cursor.readArray<uint64_t>
        (
         (m_summary.ImportMapOffset - cursor.tell()) / sizeof(uint64_t));

    cursor = m_summary.ImportMapOffset;
    m_importMap =
        cursor.readArray<FPackageObjectIndex>
        (
         m_summary.importCount());
  }

  void UAssetParser::parseExports(ByteCursor &cursor) {
    cursor = m_summary.ExportMapOffset;
    m_exportMap =
        cursor.readArray<FExportMapEntry>(m_summary.exportCount());
    for (int i = 0; i < m_exportMap.size(); i++) { m_exportHashToIndex[m_exportMap[i].PublicExportHash] = i; }
  }

  void UAssetParser::parseExportBundles(ByteCursor &cursor) {
    cursor = m_cellOffsets.CellImportMapOffset;

    m_exportBundleEntries =
        cursor.readArray<FExportBundleEntry>
        (
         m_summary.exportCount() * 2);
  }

  void UAssetParser::loadImportedPackages() {
    m_importedParsers.reserve(m_storeEntry.importedPackage.size());
    for (const auto &pkgId: m_storeEntry.importedPackage) {
      const auto parser = m_assetParser->loadPackage(pkgId);
      m_importedParsers.emplace_back(parser);
    }
  }

  void UAssetParser::processExports() {
    m_exportObjects.resize(m_exportMap.size());
    for (const auto &entry: m_exportBundleEntries) { processExportEntry(entry); }
  }

  void UAssetParser::loadExports() const {
    for (const auto &exportObj: m_exportObjects) { if (auto obj = exportObj.lock()) { obj->safeParse(); } }
  }

  void UAssetParser::exportAssets() const {
    for (const auto &exportObj: m_exportObjects) { if (auto obj = exportObj.lock()) { obj->exportAsset(); } }
  }

  void UAssetParser::reset() {
    m_summary = {};
    m_version = {};
    m_version = {};
    m_nameMap.clear();
    m_storeEntry = {};
    m_bulkDataMap = {};
    m_importMap = {};
    m_exportMap = {};
    m_exportBundleEntries = {};
    m_importedPublicExportHashes = {};
    m_exportHashToIndex.clear();
    m_exportObjects.clear();
    m_exportObjectsKeepalive.clear();
    AssetFileParser::reset();
  }

  void UAssetParser::processExportEntry(const FExportBundleEntry &entry) {
    if (entry.CommandType != EExportCommandType::Serialize) { return; }
    auto &exportMapEntry = m_exportMap[entry.LocalExportIndex];
    if (auto obj = resolveObject(exportMapEntry, exportMapEntry.ClassIndex).lock()) {
      ByteCursor cursor = getAsset().subspan
          (0, m_summary.HeaderSize + exportMapEntry.CookedSerialOffset + exportMapEntry.CookedSerialSize);
      cursor += m_summary.HeaderSize + exportMapEntry.CookedSerialOffset;
      cursor.setAbsoluteOffset(m_summary.CookedHeaderSize - m_summary.HeaderSize);
      obj->setCursor(cursor);
      m_exportObjects[entry.LocalExportIndex] = obj;
    }
  }

  std::weak_ptr<Object> UAssetParser::resolveObject(const FExportMapEntry &entry, const FPackageObjectIndex &index) {
    switch (index.GetType()) {
      case EType::Export:
        return resolveExportObject(entry, index);
      case EType::ScriptImport:
        return resolveScriptImport(entry, index);
      case EType::PackageImport:
        return resolvePackageImport(entry, index);
      default:
        return {};
    }
  }

  std::weak_ptr<Object> UAssetParser::resolveExportObject
  (const FExportMapEntry &entry, const FPackageObjectIndex &index) {
    if (index.TypeAndId >= m_exportObjects.size()) { return {}; }
    return m_exportObjects[index.TypeAndId];
  }

  std::weak_ptr<Object> UAssetParser::resolveScriptImport
  (const FExportMapEntry &entry, const FPackageObjectIndex &index) {
    const auto &scriptEntry = globalData()->ScriptObjectEntriesMap.at(index);
    auto objName = getMappedName(scriptEntry.ObjectName);
    auto obj = ObjectFactory::Create(objName);
    //Logger::logInfo("[UAssetParser]", "OBJ: {}", objName);

    if (!obj) { obj = std::make_shared<Object>(); }
    m_exportObjectsKeepalive.emplace_back(obj);
    if (obj->getClassName() == "Struct") {
      auto ustruct = std::static_pointer_cast<Struct>(obj);
      ByteCursor cursor = getAsset();
      cursor += m_summary.HeaderSize + entry.CookedSerialOffset;
      ustruct->setCursor(cursor);
      return constructObject(entry, ustruct);
    }
    obj->setObjName(getMappedName(entry.ObjectName));
    return obj;
  }

  std::weak_ptr<Object> UAssetParser::resolvePackageImport
  (const FExportMapEntry &entry, const FPackageObjectIndex &index) const {
    auto packageImportRef = FPackageImportReference::fromObjIndex(index);
    if (packageImportRef.importedPackageIndex < m_importedParsers.size()) {
      auto const &hash = m_importedPublicExportHashes[packageImportRef.importedPublicExportHashIndex];

      if (auto parser = m_importedParsers[packageImportRef.importedPackageIndex]) {
        const auto &obj = parser->resolvePackageExport(hash);
        return obj;
      }
    }

    return {};
  }

  std::weak_ptr<Object> UAssetParser::resolvePackageExport
  (const uint64_t &publicExportHash) {
    safeParse();
    auto it = m_exportHashToIndex.find(publicExportHash);
    if (it != m_exportHashToIndex.end()) { return m_exportObjects[it->second]; }
    return {};
  }

  std::weak_ptr<Object> UAssetParser::constructObject
  (const FExportMapEntry &entry, const std::shared_ptr<Struct> &ustruct) {
    ustruct->safeParse();
    if (ustruct->m_superStructIndex < 0 || (-ustruct->m_superStructIndex - 1) >= m_importMap.size()) {
      return resolveObject(entry, m_importMap[-ustruct->m_superStructIndex - 1]);
    }
    if (ustruct->m_superStructIndex > 0 && ustruct->m_superStructIndex <= m_exportMap.size()) {
      return ustruct->m_owner->resolveExportObject(entry, m_exportMap[ustruct->m_superStructIndex - 1].ClassIndex);
    }
    return {};
  }

  std::vector<std::shared_ptr<Object> > &UAssetParser::exportMap() { return m_exportObjectsKeepalive; }

  const FZenPackageSummary &UAssetParser::getSummary() const { return m_summary; }

  const std::vector<std::string_view> &UAssetParser::getNameMap() const { return m_nameMap; }
}
