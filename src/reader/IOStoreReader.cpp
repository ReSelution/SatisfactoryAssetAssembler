#include "IOStoreReader.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <span>
#include <unordered_map>
#include <vector>

#include "byte_cursor.hpp"
#include "Reader.hpp"
#include "Decompressor.hpp"
#include "logger.hpp"
#include "functions.hpp"
#include "core/constants.hpp"
#include "io/f_io_directory_index_enty.hpp"
#include "io/f_io_file_index_entry.hpp"
#include "io/f_on_demand_toc_entry.hpp"

#include "vfs.hpp"

namespace SAP {
  IOStoreReader::IOStoreReader(std::filesystem::path utocPath, const std::filesystem::path &ucasPath)
    : Reader(Reader::Type::IO_STORE, std::move(utocPath)), m_ucasReader(ucasPath) {}

  bool IOStoreReader::load() {

    auto start = std::chrono::high_resolution_clock::now();

    utocBuffer = loadFile(m_Path);
    ByteCursor cursor(utocBuffer);

    parseHeader(cursor);

    // 2. Chunk IDs
    parseChunkIds(cursor);

    // 3. Offsets & Lengths
    parseOffsetLengths(cursor);

    createChunkLookupTable();

    // 4. Perfect Hash Map
    parsePerfectHashTable(cursor);

    // 5. Compression Blocks
    parseCompressionBlocks(cursor);

    // 6. Kompressions-Methodennamen
    parseCompressionMethods(cursor);

    // 7. Chunk Data List
    parseSignatures(cursor);

    // 8. Directory Index
    parseDirectoryIndex(cursor);

    parseDirectoryPathList();

    parseTocMetas(cursor);

    if (m_Path.filename() != "global.utoc")
      readContainerHeader();

    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[IOSTORE]", "Loaded {} in {} ms", m_Path.string(),us.count() / 1000.0);
    return true;
  }

  bool IOStoreReader::mountVFS(Vfs *vfs) {
    auto start = std::chrono::high_resolution_clock::now();

    if (m_DirectoryIndexBuffer.empty() || m_DirectoryPaths.empty()) {
      Logger::logWarn("[VFS]", "Directory Index oder Pfade leer.");
      return false;
    }

    ByteCursor cursor(m_DirectoryIndexBuffer);

    auto mountPoint = cursor.readFString(m_stringStorage);

    if (mountPoint.starts_with("../../..")) {
      mountPoint = mountPoint.substr(9); // Relativen Pfad entfernen
    }

    auto dirEntries = cursor.readArray<FIoDirectoryIndexEntry>();

    auto fileEntries = cursor.readArray<FIoFileIndexEntry>();

    vfs->reserve(fileEntries.size());
    std::string pathBuffer;
    pathBuffer.reserve(512); //
    pathBuffer = mountPoint;

    auto traverse = [&](auto &self, uint32_t dirIndex) -> void {
      const auto &dir = dirEntries[dirIndex];
      size_t oldLen = pathBuffer.size(); // Merken, wo wir angefangen haben

      if (dir.name < m_DirectoryPaths.size()) {
        std::string_view dName = m_DirectoryPaths[dir.name];
        if (!dName.empty()) {
          if (!pathBuffer.empty() && pathBuffer.back() != '/') pathBuffer += '/';
          pathBuffer += dName;
        }
      }

      uint32_t fileIndex = dir.firstFileEntry;
      while (fileIndex != FIoDirectoryIndexResource_InvalidIndex) {
        const auto &file = fileEntries[fileIndex];

        size_t pathWithDirLen = pathBuffer.size();
        if (!pathBuffer.empty() && pathBuffer.back() != '/') pathBuffer += '/';
        pathBuffer += m_DirectoryPaths[file.name];

        vfs->mountEntry(this, pathBuffer, m_ChunkIds[file.userData]);

        pathBuffer.resize(pathWithDirLen);
        fileIndex = file.nextFileEntry;
      }

      uint32_t childIndex = dir.firstChildEntry;
      while (childIndex != FIoDirectoryIndexResource_InvalidIndex) {
        self(self, childIndex);
        childIndex = dirEntries[childIndex].nextSiblingEntry;
      }

      pathBuffer.resize(oldLen);
    };

    // Starten bei Root (Index 0) mit dem MountPoint als Basis
    if (dirEntries.size() > 0) { traverse(traverse, 0); }

    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[IOSTORE]", "{}: Mounted {} in {} ms", m_Path.string(),fileEntries.size() ,us.count() / 1000.0);

    return true;
  }

  void IOStoreReader::parseHeader(ByteCursor &cursor) {
    m_TocHeader = cursor.read<FIoStoreTocHeader>();
    if (memcmp
        (m_TocHeader.tocMagic.data(), EXPECTED_TOC_MAGIC.data(), EXPECTED_TOC_MAGIC.size()) != 0) {
      throw std::runtime_error("Wrong magic number.");
    }
    cursor = m_TocHeader.tocHeaderSize;
  }

  void IOStoreReader::parseChunkIds(ByteCursor &cursor) {
    m_ChunkIds = cursor.readArray<FIoChunkId>(m_TocHeader.tocEntryCount);
  }

  void IOStoreReader::parseOffsetLengths(ByteCursor &cursor) {
    m_ChunkOffsetLengths.reserve(m_TocHeader.tocEntryCount);

    auto buffer = cursor.readArray<FIoOffsetAndLength_raw>(m_TocHeader.tocEntryCount);
    for (auto raw: buffer) { m_ChunkOffsetLengths.emplace_back(FIoOffsetAndLength::get(raw)); }
  }

  bool operator>=(EIoStoreTocVersion lhs, uint8_t rhs) { return static_cast<uint8_t>(lhs) >= rhs; }

  void IOStoreReader::parsePerfectHashTable(ByteCursor &cursor) {
    m_ChunkPerfectHashSeeds = cursor.readArray<int32_t>(m_TocHeader.tocChunkPerfectHashSeedsCount);
    m_ChunkIndicesWithoutPerfectHash = cursor.readArray<uint32_t>(m_TocHeader.tocChunksWithoutPerfectHashCount);
  }

  void IOStoreReader::parseCompressionBlocks(ByteCursor &cursor) {
    auto buffer = cursor.readArray<FIoStoreTocCompressedBlockEntry_raw>
        (m_TocHeader.tocCompressedBlockEntryCount);
    m_CompressionBlocks.reserve(m_TocHeader.tocCompressedBlockEntryCount);

    for (auto &raw: buffer) { m_CompressionBlocks.emplace_back(FIoStoreTocCompressedBlockEntry::get(raw)); }
  }

  void IOStoreReader::parseCompressionMethods(ByteCursor &cursor) {
    uint32_t count = m_TocHeader.compressionMethodNameCount;
    uint32_t length = m_TocHeader.compressionMethodNameLength;

    auto dataSize = count * length;
    auto buffer = cursor.readArray<uint8_t>(dataSize);

    const uint8_t *methodBufferStart = buffer.data();
    m_CompressionMethods.resize(count + 1);

    for (uint32_t i = 0; i < count; i++) {
      auto strStart = i * length;
      std::string methodName = (std::string(reinterpret_cast<const char *>(methodBufferStart + strStart), length));
      size_t nullPos = methodName.find('\0');
      if (nullPos != std::string::npos) { methodName.resize(nullPos); }
      SAP::CompressionMethod methodEnum = stringToCompressionMethod(methodName);
      m_CompressionMethods[i + 1] = methodEnum;
    }
  }

  void IOStoreReader::parseSignatures(ByteCursor &cursor) {
    if (!(m_TocHeader.containerFlags & EIoContainerFlags::SIGNED)) { return; }

    auto hashSize = cursor.read<int32_t>();

    auto tocAndBlockSignatureSize = hashSize * 2;
    auto chunkBlockSignatureSize = FSHA_HASH_SIZE * m_TocHeader.tocCompressedBlockEntryCount;
    cursor += tocAndBlockSignatureSize + chunkBlockSignatureSize;
  }

  bool operator<(EIoStoreTocVersion lhs, uint32_t rhs) { return static_cast<uint8_t>(lhs) < rhs; }
  bool operator>(EIoStoreTocVersion lhs, uint32_t rhs) { return static_cast<uint8_t>(lhs) > rhs; }

  void IOStoreReader::parseDirectoryIndex(ByteCursor &cursor) {
    const uint32_t TOC_VERSION_DIRECTORY_INDEX = 5;
    const EIoContainerFlags CONTAINER_FLAG_INDEXED = EIoContainerFlags::INDEXED;
    assert(m_TocHeader.version >= TOC_VERSION_DIRECTORY_INDEX && "TOC version is to Low");
    assert
    ((m_TocHeader.containerFlags & CONTAINER_FLAG_INDEXED) || m_TocHeader.directoryIndexSize == 0 &&
     "No Directory Index");

    m_DirectoryIndexBuffer = cursor.readArray<uint8_t>(m_TocHeader.directoryIndexSize);
  }

  void IOStoreReader::parseTocMetas(ByteCursor &cursor) {
    uint32_t entryCount = m_TocHeader.tocEntryCount;

    auto totalHashSize = entryCount * FSHA_HASH_SIZE;

    cursor += totalHashSize;
    if (m_TocHeader.version >= TOC_VERSION_ONDEMAND_METADATA &&
        (m_TocHeader.containerFlags & EIoContainerFlags::ON_DEMAND)) {
      size_t entrySize = sizeof(FOnDemandTocEntry);
      size_t totalOnDemandSize = (size_t) entryCount * entrySize;
      cursor += totalOnDemandSize;
    }
  }

  bool IOStoreReader::readContainerHeader() {
    FIoChunkId chunkId{.id = m_TocHeader.containerId.id, .chunkIndex = 0, .chunkType = 6};
    FIoContainerHeader containerHeader{.version = EIoContainerHeaderVersion::Initial, .buffer = readChunk(chunkId) };

    ByteCursor cursor(containerHeader.buffer);

    containerHeader.magic = cursor.read<uint32_t>();
    if (containerHeader.magic != IO_CONTAINER_MAGIC) {
      Logger::logError("[IOStore]", "Wrong container magic. Found {:x}", containerHeader.magic);
      return false;
    }

    containerHeader.version = cursor.read<EIoContainerHeaderVersion>();
    containerHeader.containerId = cursor.read<FPackageId>();

    if (containerHeader.version < EIoContainerHeaderVersion::OptionalSegmentPackages) { cursor += sizeof(uint32_t); }
    containerHeader.packageIds = cursor.readArray<FPackageId>();

    auto storeEntriesEnd = cursor.read<uint32_t>() + cursor.tell();

    containerHeader.storeEntriesById.reserve(containerHeader.packageIds.size());
    for (uint32_t i = 0; i < containerHeader.packageIds.size(); ++i) {
      containerHeader.storeEntriesById.emplace
          (containerHeader.packageIds[i], readFilePackageStoreEntry(cursor, containerHeader.version));
    }
    cursor = storeEntriesEnd;

    if (containerHeader.version >= EIoContainerHeaderVersion::OptionalSegmentPackages) {
      containerHeader.optionalSegmentPackageIds = cursor.readArray<FPackageId>();

      auto optStoreEntriesEnd = cursor.read<uint32_t>() + cursor.tell();

      containerHeader.optionalSegmentStoreEntries.reserve(containerHeader.optionalSegmentPackageIds.size());
      for (uint32_t i = 0; i < containerHeader.optionalSegmentPackageIds.size(); ++i) {
        containerHeader.optionalSegmentStoreEntries.emplace_back
            (readFilePackageStoreEntry(cursor, containerHeader.version));
      }

      cursor = optStoreEntriesEnd;
    }

    if (containerHeader.version >= EIoContainerHeaderVersion::Initial) {
      containerHeader.containerNameMap = cursor.loadNameBatch(m_stringStorage);
    }

    if (containerHeader.version >= EIoContainerHeaderVersion::LocalizedPackages) {
      containerHeader.localizedPackages = cursor.readArray<FIoContainerHeaderLocalizedPackage>();
    }

    containerHeader.packageRedirects = cursor.readArray<FIoContainerHeaderPackageRedirect>();

    this->m_containerHeader = std::move(containerHeader);
    return true;
  }

  FFilePackageStoreEntry IOStoreReader::readFilePackageStoreEntry
  (ByteCursor &cursor, EIoContainerHeaderVersion version) {
    FFilePackageStoreEntry entry{};
    if (version >= EIoContainerHeaderVersion::Initial) {
      if (version < EIoContainerHeaderVersion::NoExportInfo) {
        entry.exportCount = cursor.read<uint32_t>();
        entry.exportBundleCount = cursor.read<uint32_t>();
      }

      entry.importedPackage = cursor.readArrayView<FPackageId>();
      entry.shaderMapHashes = cursor.readArrayView<FSHAHash>();
    } else {
      cursor += 8; // ExportBundlesSize (skip 8)
      entry.exportCount = cursor.read<uint32_t>();
      entry.exportBundleCount = cursor.read<uint32_t>();
      cursor += 8; // LoadOrder + Pad
      entry.importedPackage = cursor.readArrayView<FPackageId>();
    }
    return entry;
  }

  FIoContainerHeaderSoftPackageReferences IOStoreReader::readSoftPackageReferences
  (ByteCursor &cursor) {
    FIoContainerHeaderSoftPackageReferences out;

    out.bContainsSoftPackageReferences = cursor.readBool();

    if (out.bContainsSoftPackageReferences) {
      out.PackageIds = cursor.readArray<FPackageId>();
      out.PackageIndices = cursor.readArray<uint8_t>();
    }

    return out;
  }

  void IOStoreReader::parseDirectoryPathList() {
    if (m_DirectoryIndexBuffer.empty()) { return; }

    ByteCursor cursor(m_DirectoryIndexBuffer);
    auto mountPoint = cursor.readFString(m_stringStorage);

    auto dirEntryCount = cursor.read<uint32_t>();
    cursor += dirEntryCount * sizeof(FIoDirectoryIndexEntry); // Skip we only want the strings

    auto fileEntryCount = cursor.read<uint32_t>();

    cursor += fileEntryCount * sizeof(FIoFileIndexEntry); //Skip

    auto stringCount = cursor.read<uint32_t>();

    Logger::logInfo("[Path List]", "Lese {} Pfad-Strings...", stringCount);

    m_DirectoryPaths.reserve(stringCount);
    for (uint32_t i = 0; i < stringCount; ++i) { m_DirectoryPaths.emplace_back(cursor.readFString(m_stringStorage)); }
  }

  void IOStoreReader::createChunkLookupTable() {
    if (m_ChunkIds.size() != m_ChunkOffsetLengths.size()) {
      Logger::logError
          ("[Lookup Table]",
           "Chunk IDs ({}) und Offsets ({}) stimmen in der Anzahl nicht überein!",
           m_ChunkIds.size(),
           m_ChunkOffsetLengths.size());
      return;
    }

    m_ImperfectChunkOffsetMap.reserve(m_ChunkIds.size()); // Optional, aber gut für die Performance
    for (size_t i = 0; i < m_ChunkIds.size(); ++i) {
      m_ImperfectChunkOffsetMap[m_ChunkIds[i]] = m_ChunkOffsetLengths[i];
    }
  }

  inline uint64_t hashChunk(uint32_t seed, const FIoChunkId *chunkId) {
    const auto *bytes = reinterpret_cast<const uint8_t *>(chunkId);
    constexpr size_t datasize = sizeof(FIoChunkId);
    uint64_t hash = seed != 0 ? static_cast<uint64_t>(seed) : 0xcbf29ce484222325;
    for (size_t i = 0; i < datasize; ++i) { hash = (hash * 0x00000100000001B3) ^ bytes[i]; }
    return hash;
  }

  bool IOStoreReader::resolveChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const {
    if (m_TocHeader.tocEntryCount == 0) {
      outOffsetLength = {};
      return false;
    }

    auto seedCount = m_ChunkPerfectHashSeeds.size();
    auto seedIndex = hashChunk(0, &chunkId) % seedCount;
    auto seed = m_ChunkPerfectHashSeeds[seedIndex];
    if (seed == 0) {
      outOffsetLength = {};
      return false;
    }
    uint32_t slot;
    if (seed < 0) {
      uint32_t seedAsIndex = -seed - 1;
      if (seedAsIndex < m_TocHeader.tocEntryCount) { slot = seedAsIndex; } else {
        return resolveImperfectChunk(chunkId, outOffsetLength);
      }
    } else { slot = hashChunk(seed, &chunkId) % m_TocHeader.tocEntryCount; }

    if (m_ChunkIds[slot] == chunkId) {
      outOffsetLength = m_ChunkOffsetLengths[slot];
      return true;
    }
    outOffsetLength = {};
    return false;
  }

  bool IOStoreReader::resolveImperfectChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const  {
    auto iter = m_ImperfectChunkOffsetMap.find(chunkId);
    if (iter == m_ImperfectChunkOffsetMap.end()) {
      outOffsetLength = {};
      return false;
    }
    outOffsetLength = iter->second;
    return true;
  }

  std::vector<uint8_t> IOStoreReader::readChunk(const FIoChunkId &chunkId) const{
    std::vector<uint8_t> uncompressedData;

    FIoOffsetAndLength offsetLenght{};
    if (!resolveChunk(chunkId, offsetLenght)) {
      Logger::logError("[Chunk Read]", "Chunk {} nicht gefunden.", chunkId.chunkIndex);
      return uncompressedData;
    }

    uint64_t index = offsetLenght.offset / m_TocHeader.compressionBlockSize;
    const uint64_t lenght = offsetLenght.length;
    uncompressedData.resize(lenght);
    auto uncompressedSpan = std::span(uncompressedData);
    for (uint64_t offset = 0; offset < lenght; index++) {
      auto &block = m_CompressionBlocks[index];
      auto dst = uncompressedSpan.subspan(offset, block.uncompressedSize);
      bool success = readBlock(block, dst);
      if (!success) {
        uncompressedData.clear();
        Logger::logError("[Chunk Read]", "Fehler beim Lesen oder Dekomprimieren von Block {}.", index);
        return uncompressedData;
      }
      offset += block.uncompressedSize;
    }
    return uncompressedData;
  }

  bool IOStoreReader::readBlock(const FIoStoreTocCompressedBlockEntry &blockEntry, std::span<uint8_t> dst) const {
    uint64_t compressedOffset = blockEntry.compressedOffset;
    uint32_t compressedSize = blockEntry.compressedSize;
    uint8_t methodIndex = blockEntry.compressionMethod;

    CompressionMethod compressionMethod;
    if (methodIndex < m_CompressionMethods.size()) { compressionMethod = m_CompressionMethods[methodIndex]; } else {
      Logger::logError("[IOStore]", "Unbekannter Kompressionsindex {} gefunden.", methodIndex);
      return false;
    }

    std::span<const uint8_t> compressedData = m_ucasReader.getSpan(compressedOffset, compressedSize);
    return Decompressor::decompress(compressedData, dst, compressionMethod);
  }
} // namespace SAP
