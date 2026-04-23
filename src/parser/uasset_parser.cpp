#include "uasset_parser.hpp"

#include <algorithm>

#include "AssetParser.hpp"
#include "IOStoreReader.hpp"
#include "io_global_data.hpp"
#include "../assets/object_factory.hpp"
#include "vfs_entry.hpp"

#include "io/packages/f_zen_package_versioning_info.hpp"
#include "logger/chaos_logger.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

LOG_ALIAS(uLOG, "SAA", "UASSET")
DEFINE_CHAOS_STAT(ObjThrouput, "OBJs", SC::ChaosThroughput<SC::MetricUnits>)
REGISTER_CHAOS_STAT(ObjThrouput)

namespace SAA {
  UAssetParser::UAssetParser(AssetParser *assetParser, const IOVfsEntry *vfsEntry) : AssetFileParser(
    assetParser, vfsEntry->pathStr, AssetType::UASSET) {
    auto reader = vfsEntry->owningReader;
    m_containerHeader = reader->containerHeader();
  }

  void UAssetParser::parse() {
    auto cursor = getAssetFile();

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


  FByteBulkData *UAssetParser::parseFByteBulkData(ByteCursor &cursor) {
    FByteBulkData *data = storage()->make<FByteBulkData>();
    auto dataIndex = cursor.read<int32_t>();
    if (dataIndex >= m_bulkDataMap.size()) {
      return data;
    }
    auto &metaData = m_bulkDataMap[dataIndex];
    data->flags = static_cast<EBulkDataFlags>(metaData.Flags);
    if (HasFlag(data->flags, EBulkDataFlags::BULKDATA_ForceInlinePayload)) {
      data->bulkCursor = cursor.subspan(cursor.tell(), metaData.SerialSize);
      cursor += metaData.SerialSize;
    } else if (HasFlag(data->flags, EBulkDataFlags::BULKDATA_PayloadInSeperateFile)) {
      auto bulkFile = getBulkFile();
      data->bulkCursor = bulkFile->subspan(metaData.SerialOffset, metaData.SerialSize);
    }

    return data;
  }

  Object *UAssetParser::resolveIndex(int32_t index) {
    Object *ret = nullptr;

    if (index < 0 && (-index - 1) < m_importMap.size()) {
      ret =  resolveObject({}, m_importMap[(-index - 1)], false);

    }
    else if (index > 0 && index - 1 < m_exportMap.size()) {
      ret =  resolveExportObject({}, m_exportMap[index - 1].ClassIndex);
    }
    return ret;
  }

  Object *UAssetParser::resolveSoftObj(std::string_view assetName) {
    safeParse();
    if (auto iter = m_nameToIndex.find(assetName); iter != m_nameToIndex.end()){
      return m_exportObjects[iter->second];
    }
    return nullptr;
  }

  void UAssetParser::parseSummary(GameFile * file) { m_summary = file->read<FZenPackageSummary>(); }

  void UAssetParser::parseVersioning(GameFile* file) {
    m_version = mappings()->pkgVersion;

    if (m_summary.bHasVersioningInfo != 0) [[unlikely]] {
      const auto info = file->read<FZenPackageVersioningInfo>();
      m_version = info.PackageVersion;
    }
  }

  void UAssetParser::parseCellOffsets(GameFile * file) {
    if (m_summary.bHasVersioningInfo == 0 && m_version.FileVersionUE5 >= EUnrealEngineObjectUE5Version::VERSE_CELLS) {
      m_cellOffsets = file->read<FZenPackageCellOffsets>();
    } else {
      m_cellOffsets.CellImportMapOffset = m_summary.ExportBundleEntriesOffset;
      m_cellOffsets.CellExportMapOffset = m_summary.ExportBundleEntriesOffset;
    }
  }

  void UAssetParser::parseNameMap(GameFile * file) { m_nameMap = file->loadNameBatch(); }

  void UAssetParser::resolveStoreEntry() {
    std::string_view name = getMappedName(m_summary.Name);

    const auto packageId = FPackageId::fromName(name);
    auto entry = m_containerHeader->storeEntriesById.find(packageId);
    if (entry != m_containerHeader->storeEntriesById.end()) {
      m_storeEntry = entry->second;
    }
  }

  void UAssetParser::parseBulkDataMap(GameFile * file) {
    if (m_version.FileVersionUE5 < EUnrealEngineObjectUE5Version::DATA_RESOURCES) {
      return;
    }

    if (m_version.FileVersionUE5 >= EUE5Version::UE5_4) {
      auto padding = file->read<uint64_t>();
      *file += padding;
    }

    const auto mapSize = file->read<uint64_t>();

    m_bulkDataMap = file->readArray<FBulkDataMapEntry>(mapSize / sizeof(FBulkDataMapEntry));
  }

  void UAssetParser::parseImports(GameFile * file) {
    *file = m_summary.ImportedPublicExportHashesOffset;

    m_importedPublicExportHashes =
        file->readArray<uint64_t>((m_summary.ImportMapOffset - file->tell()) / sizeof(uint64_t));

    *file = m_summary.ImportMapOffset;
    m_importMap = file->readArray<FPackageObjectIndex>(m_summary.importCount());
  }

  void UAssetParser::parseExports(GameFile * file) {
    *file = m_summary.ExportMapOffset;
    m_exportMap = file->readArray<FExportMapEntry>(m_summary.exportCount());
    m_exportHashToIndex = exportDenseMap(m_storage->resource());
    m_exportHashToIndex.reserve(m_exportMap.size());
    for (int i = 0; i < m_exportMap.size(); i++) {
      m_exportHashToIndex.emplace(m_exportMap[i].PublicExportHash, i);
    }
  }

  void UAssetParser::parseExportBundles(GameFile * file) {
    *file = m_cellOffsets.CellImportMapOffset;

    m_exportBundleEntries = file->readArray<FExportBundleEntry>(m_summary.exportCount() * 2);
  }

  void UAssetParser::loadImportedPackages() {

    m_importedParsers =  m_storage->make<std::pmr::vector<std::shared_ptr<AssetFileParser> >>(m_storage->resource());
    m_importedParsers->reserve(m_storeEntry.importedPackage.size());
    // std::vector<std::string_view> paths;
    // paths.reserve(m_storeEntry.importedPackage.size());
    for (const auto &pkgId: m_storeEntry.importedPackage) {
      const auto &parser = m_assetParser->loadPackage(pkgId);
      m_importedParsers->emplace_back(parser);
      // paths.emplace_back(parser->path());
    }
    // std::ranges::sort(paths);
    // for (const auto &path: paths) {
    //   uLOG::info("PATH: {}", path);
    // }
  }


  void UAssetParser::processExports() {
    m_exportObjects = m_storage->allocateSpan<Object *>(m_exportMap.size());
    std::memset(m_exportObjects.data(), 0, m_exportObjects.size_bytes());
    m_nameToIndex = nameDenseMap (m_storage->resource());
    m_nameToIndex.reserve(m_exportMap.size());
    for (const auto &entry: m_exportBundleEntries) {
      processExportEntry(entry);
    }
    CHAOS_RECORD(ObjThrouput,m_exportMap.size() )

  }

  void UAssetParser::loadExports() const {
    for (auto obj: m_exportObjects) {
      if (obj) {
        obj->safeParse();
      }
    }
  }

  void UAssetParser::exportAssets() const {
    for (auto obj: m_exportObjects) {
      if (obj) {
        obj->safeExport();
      }
    }
  }

  void UAssetParser::reset() {
    m_summary = {};
    m_version = {};
    m_version = {};
    m_nameMap = {};
    m_storeEntry = {};
    m_bulkDataMap = {};
    m_importMap = {};
    m_exportMap = {};
    m_exportBundleEntries = {};
    m_importedPublicExportHashes = {};
    m_exportHashToIndex.clear();
    m_exportObjects = {};
    AssetFileParser::reset();
  }

  void UAssetParser::processExportEntry(const FExportBundleEntry &entry) {
    if (entry.CommandType != EExportCommandType::Create) {
      return;
    }
    auto &exportMapEntry = m_exportMap[entry.LocalExportIndex];
    if (auto obj = resolveObject(exportMapEntry, exportMapEntry.ClassIndex, true)) [[likely]] {
      obj->setFileSpan(m_summary.HeaderSize + exportMapEntry.CookedSerialOffset, exportMapEntry.CookedSerialSize );
      obj->outer(resolveObject(exportMapEntry, exportMapEntry.OuterIndex, false));
      obj->super(resolveObject(exportMapEntry, exportMapEntry.SuperIndex, false));
      m_exportObjects[entry.LocalExportIndex] = obj;
      m_nameToIndex[obj->objName()] = entry.LocalExportIndex;
    }
  }

  Object *UAssetParser::resolveObject(const FExportMapEntry &entry, const FPackageObjectIndex &index, bool copy) {
    switch (index.GetType()) {
      case EType::Export:
        return resolveExportObject(entry, index);
      case EType::ScriptImport:
        return resolveScriptImport(entry, index, copy);
      case EType::PackageImport:
        return resolvePackageImport(entry, index, copy);
      default:
        return nullptr;
    }
  }

  Object *UAssetParser::resolveExportObject(const FExportMapEntry &entry,
                                            const FPackageObjectIndex &index) {
    if (index.TypeAndId >= m_exportObjects.size()){
      return nullptr;
    }
    auto obj = m_exportObjects[index.TypeAndId];
    return obj;
  }

  Object *UAssetParser::resolveScriptImport(const FExportMapEntry &entry,
                                            const FPackageObjectIndex &index, bool copy) {

    const auto &scriptEntry = globalData()->ScriptObjectEntriesMap.at(index);
    auto className = getMappedName(scriptEntry.ObjectName);
    auto objName = getMappedName(entry.ObjectName);
    auto obj = ObjectFactory::Create(className, m_storage.get());
    //Logger::logInfo("[UAssetParser]", "{} -> {}", objName, clasName);

    if (!obj) {
      obj = m_storage->make<Object>(className);
    }
    obj->setObjName(objName);
    obj->setOwner(this);

    if (obj->getClassHash() == Struct::StaticHASH) {
      auto ustruct = static_cast<Struct *>(obj);
      ustruct->setFileSpan( m_summary.HeaderSize + entry.CookedSerialOffset, entry.CookedSerialSize);
      return constructObject(entry, ustruct, copy);
    }

    return obj;
  }

  Object *UAssetParser::resolvePackageImport(const FExportMapEntry &entry,
                                             const FPackageObjectIndex &index, bool copy) {
    auto packageImportRef = FPackageImportReference::fromObjIndex(index);
    if (packageImportRef.importedPackageIndex < m_importedParsers->size()) {
      auto const &hash = m_importedPublicExportHashes[packageImportRef.importedPublicExportHashIndex];

      if (auto parser = m_importedParsers->at(packageImportRef.importedPackageIndex)) {

        auto obj = parser->resolvePackageExport(hash);
        if (! obj || !copy) {
          return obj;
        }
        auto className = obj->getClassName();
        obj = ObjectFactory::Create(className, m_storage.get());
        if (!obj) {
          obj = m_storage->make<Object>(className);
        }
        obj->setOwner(this);
        return obj;
      }
    }

    return nullptr;
  }

  Object *UAssetParser::resolvePackageExport(const uint64_t &publicExportHash) {
    safeParse();
    auto it = m_exportHashToIndex.find(publicExportHash);
    if (it != m_exportHashToIndex.end()) {
      return m_exportObjects[it->second];
    }
    return nullptr;
  }

  Object *UAssetParser::constructObject(const FExportMapEntry &entry,
                                        Struct *ustruct, bool copy) {
    ustruct->safeParse();
    if (ustruct->m_superStructIndex < 0 || (-ustruct->m_superStructIndex - 1) <= m_importMap.size()) {
      return resolveObject(entry, m_importMap[-ustruct->m_superStructIndex - 1], copy);
    }
    if (ustruct->m_superStructIndex > 0 && ustruct->m_superStructIndex <= m_exportMap.size()) {
      return this->resolveExportObject(entry, m_exportMap[ustruct->m_superStructIndex - 1].ClassIndex);
    }
    return nullptr;
  }

  std::span<Object *> UAssetParser::exportMap() { return m_exportObjects; }

  const FZenPackageSummary &UAssetParser::getSummary() const { return m_summary; }

  std::span<std::string_view> UAssetParser::getNameMap() const { return m_nameMap; }
}
