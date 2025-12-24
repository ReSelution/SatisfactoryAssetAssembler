#include "IOStoreReader.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <span>
#include <unordered_map>
#include <vector>

#include "Reader.hpp"
#include "Decompressor.hpp"
#include "logger.hpp"
#include "functions.hpp"
#include "core/constants.hpp"
#include "io/f_io_directory_index_enty.hpp"
#include "io/f_io_file_index_entry.hpp"
#include "io/f_on_demand_toc_entry.hpp"

namespace SAP {
  static constexpr size_t PARSE_ERROR = std::numeric_limits<size_t>::max();

  IOStoreReader::IOStoreReader(std::filesystem::path utocPath, std::filesystem::path ucasPath)
    : Reader(Reader::Type::IO_STORE, std::move(utocPath)), m_UcasPath(std::move(ucasPath)) {}

  IOStoreReader::~IOStoreReader() = default;

  CompressionMethod stringToCompressionMethod(const std::string &name) {
    if (name == "Zlib") { return CompressionMethod::Zlib; }
    if (name == "Gzip") { return CompressionMethod::Gzip; }
    if (name == "Custom") { return CompressionMethod::Custom; }
    if (name == "Oodle") { return CompressionMethod::Oodle; }
    if (name == "LZ4") { return CompressionMethod::LZ4; }
    if (name == "Zstd") { return CompressionMethod::Zstd; }
    if (name == "XB1Zlib") { return CompressionMethod::XB1Zlib; }
    // Default-Fall
    return CompressionMethod::Unknown;
  }

  bool IOStoreReader::load() {
    // --- Sequenzielles Parsen ---
    size_t currentOffset = 0;
    size_t bytesRead = 0;

    // 1. Header
    bytesRead = parseHeader(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset = bytesRead; // Header size ist absolut, nicht relativ, aber parseHeader gibt HeaderSize zurück

    // 2. Chunk IDs
    bytesRead = parseChunkIds(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset += bytesRead;

    // 3. Offsets & Lengths
    bytesRead = parseOffsetLengths(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset += bytesRead;
    createChunkLookupTable();

    // 4. Perfect Hash Map
    bytesRead = parsePerfectHashTable(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset += bytesRead;

    // 5. Compression Blocks
    bytesRead = parseCompressionBlocks(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset += bytesRead;

    // 6. Kompressions-Methodennamen
    bytesRead = parseCompressionMethods(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset += bytesRead;

    // 7. Chunk Data List
    bytesRead = parseSignatures(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset += bytesRead;

    // 8. Directory Index
    bytesRead = parseDirectoryIndex(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }
    currentOffset += bytesRead;

    if (!parseDirectoryPathList()) { Logger::logError("[IOStore]", "Fehler beim Parsen der Pfadliste."); }

    bytesRead = parseTocMetas(currentOffset);
    if (bytesRead == PARSE_ERROR) { return false; }

    if (m_Path.filename() == "global.utoc")
      return true;

    return readContainerHeader();
  }

  bool IOStoreReader::mountVFS(Vfs &vfs) {
    if (m_DirectoryIndexBuffer.empty() || m_DirectoryPaths.empty()) {
      Logger::logWarn("[VFS]", "Directory Index oder Pfade leer.");
      return false;
    }

    size_t cursor = 0;
    const uint8_t *bufferStart = m_DirectoryIndexBuffer.data();
    size_t bufferSize = m_DirectoryIndexBuffer.size();

    // 1. Mount Point lesen
    // Der Mount Point ist oft so etwas wie "../../../" oder "/Game/"
    std::string mountPoint = readFString(m_DirectoryIndexBuffer, cursor);

    // Bereinigen des MountPoints (optional, je nach Bedarf)
    if (mountPoint.starts_with("../../..")) {
      mountPoint = mountPoint.substr(9); // Relativen Pfad entfernen
    }

    // 2. Directory Entries
    if (cursor + sizeof(uint32_t) > bufferSize) { return false; }
    uint32_t dirEntryCount = 0;
    std::memcpy(&dirEntryCount, bufferStart + cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);

    const auto *dirEntries = reinterpret_cast<const FIoDirectoryIndexEntry *>(bufferStart + cursor);
    cursor += dirEntryCount * sizeof(FIoDirectoryIndexEntry);

    // 3. File Entries
    if (cursor + sizeof(uint32_t) > bufferSize) { return false; }
    uint32_t fileEntryCount = 0;
    std::memcpy(&fileEntryCount, bufferStart + cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);

    const auto *fileEntries = reinterpret_cast<const FIoFileIndexEntry *>(bufferStart + cursor);

    // Sicherheitscheck für Buffer-Overflow
    if (cursor + (fileEntryCount * sizeof(FIoFileIndexEntry)) > bufferSize) {
      Logger::logError("[VFS]", "Buffer Overflow erkannt.");
      return false;
    }

    vfs.reserve(fileEntryCount);

    std::function<void(uint32_t, std::string)> traverseDirectory;

    traverseDirectory = [&](uint32_t dirIndex, std::string path) {
      if (dirIndex >= dirEntryCount) { return; }

      const auto &dir = dirEntries[dirIndex];

      // A. Pfad aktualisieren
      // Name des aktuellen Ordners holen
      std::string currentDirName;
      if (dir.name != FIoDirectoryIndexResource_InvalidIndex && dir.name < m_DirectoryPaths.size()) {
        currentDirName = m_DirectoryPaths[dir.name];
      }

      // Pfad zusammenbauen: ParentPath + "/" + CurrentDirName
      std::string currentPath = std::move(path);
      if (!currentDirName.empty()) {
        // Vermeiden von doppelten Slashes, falls mountPoint schon einen hat
        if (!currentPath.empty() && currentPath.back() != '/' && currentDirName.front() != '/') { currentPath += '/'; }
        currentPath += currentDirName;
      }

      // B. Dateien in diesem Ordner verarbeiten (Linked List)
      uint32_t fileIndex = dir.firstFileEntry;
      while (fileIndex != FIoDirectoryIndexResource_InvalidIndex) {
        if (fileIndex >= fileEntryCount) { break; }

        const auto &file = fileEntries[fileIndex];

        // Dateinamen holen
        if (file.name < m_DirectoryPaths.size() && file.userData < m_ChunkIds.size()) {
          std::string fileName = m_DirectoryPaths[file.name];

          // Vollen Pfad erstellen
          std::string fullFilePath = currentPath;
          if (!fullFilePath.empty() && fullFilePath.back() != '/') { fullFilePath += '/'; }
          fullFilePath += fileName;

          // Mapping speichern

          vfs.mountEntry(this, fullFilePath, m_ChunkIds[file.userData]);
        }

        // Nächste Datei im gleichen Ordner
        fileIndex = file.nextFileEntry;
      }

      // C. Unterordner verarbeiten (Rekursion)
      // Wir gehen zum ersten Kind (First Child) und iterieren dann durch dessen Geschwister (Siblings)
      uint32_t childIndex = dir.firstChildEntry;
      while (childIndex != FIoDirectoryIndexResource_InvalidIndex) {
        traverseDirectory(childIndex, currentPath);
        if (childIndex < dirEntryCount) { childIndex = dirEntries[childIndex].nextSiblingEntry; } else { break; }
      }
    };

    // Starten bei Root (Index 0) mit dem MountPoint als Basis
    if (dirEntryCount > 0) { traverseDirectory(0, mountPoint); }

    Logger::logInfo("[VFS]", "VFS erfolgreich erstellt. {} Dateien gemountet.", fileEntryCount);
    return true;
  }

  size_t IOStoreReader::parseHeader(size_t offset) {
    if (!readRawBytes
      (offset, sizeof(FIoStoreTocHeader), reinterpret_cast<uint8_t *>(&m_TocHeader))) { return PARSE_ERROR; }
    if (memcmp
        (m_TocHeader.tocMagic.data(), EXPECTED_TOC_MAGIC.data(), EXPECTED_TOC_MAGIC.size()) != 0) {
      return PARSE_ERROR;
    }
    return m_TocHeader.tocHeaderSize;
  }

  size_t IOStoreReader::parseChunkIds(size_t offset) {
    size_t size = m_TocHeader.tocEntryCount * sizeof(FIoChunkId);
    m_ChunkIds.resize(m_TocHeader.tocEntryCount);
    if (!readRawBytes(offset, size, reinterpret_cast<uint8_t *>(m_ChunkIds.data()))) { return PARSE_ERROR; }
    return size;
  }

  size_t IOStoreReader::parseOffsetLengths(size_t offset) {
    size_t rawEntrySize = sizeof(FIoOffsetAndLength);
    size_t dataSize = m_TocHeader.tocEntryCount * rawEntrySize;

    m_ChunkOffsetLengths.resize(m_TocHeader.tocEntryCount);

    if (!readRawBytes(offset, dataSize, reinterpret_cast<uint8_t *>(m_ChunkOffsetLengths.data()))) {
      Logger::logError("[IOStore]", "Buffer overflow reading Offset/Lengths.");
      return PARSE_ERROR;
    }

    const uint8_t *currentPtr = reinterpret_cast<uint8_t *>(m_ChunkOffsetLengths.data());

    for (uint32_t i = 0; i < m_TocHeader.tocEntryCount; i++) {
      // Die 10 rohen Bytes für diesen Eintrag
      const uint8_t *rawBytes = currentPtr + (i * rawEntrySize);

      uint64_t decodedOffset = 0;
      uint64_t decodedLength = 0;

      decodedOffset |= static_cast<uint64_t>(rawBytes[0]) << 32; // Byte 0 (Index 0): Höchstes Byte
      decodedOffset |= static_cast<uint64_t>(rawBytes[1]) << 24; // Byte 1 (Index 1)
      decodedOffset |= static_cast<uint64_t>(rawBytes[2]) << 16; // Byte 2 (Index 2)
      decodedOffset |= static_cast<uint64_t>(rawBytes[3]) << 8;  // Byte 3 (Index 3)
      decodedOffset |= static_cast<uint64_t>(rawBytes[4]);       // Byte 4 (Index 4): Niedrigstes Byte

      decodedLength |= static_cast<uint64_t>(rawBytes[5]) << 32; // Byte 5 (Index 5): Höchstes Byte
      decodedLength |= static_cast<uint64_t>(rawBytes[6]) << 24; // Byte 6 (Index 6)
      decodedLength |= static_cast<uint64_t>(rawBytes[7]) << 16; // Byte 7 (Index 7)
      decodedLength |= static_cast<uint64_t>(rawBytes[8]) << 8;  // Byte 8 (Index 8)
      decodedLength |= static_cast<uint64_t>(rawBytes[9]);       // Byte 9 (Index 9): Niedrigstes Byte

      m_ChunkOffsetLengths[i].offset = decodedOffset;
      m_ChunkOffsetLengths[i].lenght = decodedLength;
    }

    return dataSize;
  }

  bool operator>=(EIoStoreTocVersion lhs, uint8_t rhs) {
    return static_cast<uint8_t>(lhs) >= rhs;
  }

  size_t IOStoreReader::parsePerfectHashTable(size_t offset) {
    size_t perfectHashSeedsCount = 0;
    size_t chunksWithoutPerfectHashCount = 0;
    size_t localOffset = 0; // Relativer Offset innerhalb dieses Blocks

    // Version Checks
    if (m_TocHeader.version >= static_cast<uint8_t>(EIoStoreTocVersion::PERFECT_HASH_WITH_OVERFLOW)) {
      perfectHashSeedsCount = m_TocHeader.tocChunkPerfectHashSeedsCount;
      chunksWithoutPerfectHashCount = m_TocHeader.tocChunksWithoutPerfectHashCount;
    } else if (m_TocHeader.version >= static_cast<uint8_t>(EIoStoreTocVersion::PERFECT_HASH)) {
      perfectHashSeedsCount = m_TocHeader.tocChunkPerfectHashSeedsCount;
    }

    // Seeds lesen
    if (perfectHashSeedsCount > 0) {
      size_t sizeInBytes = perfectHashSeedsCount * sizeof(uint32_t);
      m_ChunkPerfectHashSeeds.resize(perfectHashSeedsCount);
      if (!readRawBytes
        (
         offset + localOffset, sizeInBytes, reinterpret_cast<uint8_t *>(m_ChunkPerfectHashSeeds.data()))) {
        return PARSE_ERROR;
      }
      localOffset += sizeInBytes;
    }

    // 2. Indices ohne Perfect Hash lesen
    if (chunksWithoutPerfectHashCount > 0) {
      size_t sizeInBytes = chunksWithoutPerfectHashCount * sizeof(uint32_t);
      m_ChunkIndicesWithoutPerfectHash.resize(chunksWithoutPerfectHashCount);
      if (!readRawBytes
        (
         offset + localOffset, sizeInBytes, reinterpret_cast<uint8_t *>(m_ChunkIndicesWithoutPerfectHash.data()))) {
        return PARSE_ERROR;
      }
      localOffset += sizeInBytes;
    }
    return localOffset; // Gesamtgröße dieses Blocks zurückgeben
  }

  size_t IOStoreReader::parseCompressionBlocks(size_t offset) {
    uint32_t blockCount = m_TocHeader.tocCompressedBlockEntryCount;

    if (blockCount == 0) { return 0; }

    size_t dataSize = blockCount * sizeof(FIoStoreTocCompressedBlockEntry);
    m_CompressionBlocks.resize(blockCount);

    if (!readRawBytes
      (offset, dataSize, reinterpret_cast<uint8_t *>(m_CompressionBlocks.data()))) { return PARSE_ERROR; }

    return dataSize;
  }

  size_t IOStoreReader::parseCompressionMethods(size_t offset) {
    uint32_t count = m_TocHeader.compressionMethodNameCount;
    uint32_t length = m_TocHeader.compressionMethodNameLength;

    if (count == 0 || length == 0) { return 0; }

    size_t dataSize = (size_t) count * length;
    std::vector<uint8_t> buffer(dataSize);

    if (!readRawBytes(offset, dataSize, buffer.data())) { return PARSE_ERROR; }

    const uint8_t *methodBufferStart = buffer.data();

    m_CompressionMethods.resize(count + 1);


    for (uint32_t i = 0; i < count; i++) {
      size_t strStart = (size_t) i * length;
      std::string methodName = (std::string(reinterpret_cast<const char *>(methodBufferStart + strStart), length));
      size_t nullPos = methodName.find('\0');
      if (nullPos != std::string::npos) { methodName.resize(nullPos); }
      SAP::CompressionMethod methodEnum = stringToCompressionMethod(methodName);
      m_CompressionMethods[i + 1] = methodEnum;
      Logger::logError("[COMPRESSION_METHOD]", "methodName {}", methodName);
    }

    return dataSize;
  }

  size_t IOStoreReader::parseSignatures(size_t offset) {
    if (!(m_TocHeader.containerFlags & EIoContainerFlags::SIGNED)) { return 0; }

    size_t currentOffset = offset;

    // 2. Hash-Größe lesen (entspricht 'var hashSize = archive.Read<int>();')
    uint32_t hashSize = 0;
    if (!readRawBytes
      ((long) currentOffset, sizeof(uint32_t), reinterpret_cast<uint8_t *>(&hashSize))) { return PARSE_ERROR; }
    currentOffset += sizeof(uint32_t);

    if (hashSize == 0) {
      Logger::logWarn("[IOStore]", "HashSize ist 0, keine Signaturen zu überspringen.");
      return (currentOffset - offset);
    }

    size_t tocAndBlockSignatureSize = (size_t) hashSize * 2;

    size_t chunkBlockSignatureSize = (size_t) FSHA_HASH_SIZE * m_TocHeader.tocCompressedBlockEntryCount;

    // Gesamtgröße der zu überspringenden Daten
    size_t totalSkipSize = tocAndBlockSignatureSize + chunkBlockSignatureSize;

    // 4. Position aktualisieren
    if (fileSize() < currentOffset + totalSkipSize) {
      Logger::logError("[IOStore]", "Buffer overflow beim Überspringen der Signaturen.");
      return PARSE_ERROR;
    }
    currentOffset += totalSkipSize;
    return currentOffset - offset;
  }

  bool operator<(EIoStoreTocVersion lhs, uint32_t rhs) {
    return  static_cast<uint8_t>(lhs) < rhs;
  }

  size_t IOStoreReader::parseDirectoryIndex(size_t offset) {
    const uint32_t TOC_VERSION_DIRECTORY_INDEX = 5;
    const EIoContainerFlags CONTAINER_FLAG_INDEXED = EIoContainerFlags::INDEXED;
    if (m_TocHeader.version < TOC_VERSION_DIRECTORY_INDEX) {
      Logger::logDebug("[IOStore]", "Directory Index nicht erwartet: TOC Version zu niedrig.");
      return 0;
    }

    if (!(m_TocHeader.containerFlags & CONTAINER_FLAG_INDEXED) || m_TocHeader.directoryIndexSize == 0) {
      Logger::logDebug("[IOStore]", "Directory Index nicht vorhanden (Flags oder Size 0).");
      return 0;
    }

    uint32_t indexSize = m_TocHeader.directoryIndexSize;
    size_t currentOffset = offset;

    m_DirectoryIndexBuffer.resize(indexSize);
    if (!readRawBytes(currentOffset, indexSize, m_DirectoryIndexBuffer.data())) { return PARSE_ERROR; }

    currentOffset += indexSize;
    return currentOffset - offset;
  }

  size_t IOStoreReader::parseTocMetas(size_t offset) {
    size_t startOffset = offset;
    size_t currentOffset = offset;
    uint32_t entryCount = m_TocHeader.tocEntryCount;

    size_t hashSize = FSHA_HASH_SIZE; // 20 Bytes
    size_t totalHashSize = (size_t) entryCount * hashSize;

    if (fileSize() < currentOffset + totalHashSize) {
      Logger::logError("[IOStore]", "Buffer overflow beim Lesen der Hashes in Chunk Metas.");
      return PARSE_ERROR;
    }

    currentOffset += totalHashSize;
    if (m_TocHeader.version >= TOC_VERSION_ONDEMAND_METADATA &&
        (m_TocHeader.containerFlags & EIoContainerFlags::ON_DEMAND)) {
      size_t entrySize = sizeof(FOnDemandTocEntry);
      size_t totalOnDemandSize = (size_t) entryCount * entrySize;

      if (fileSize() < currentOffset + totalOnDemandSize) {
        Logger::logError("[IOStore]", "Buffer overflow beim Lesen der OnDemand Metadaten.");
        return PARSE_ERROR;
      }

      currentOffset += totalOnDemandSize;
    }

    return currentOffset - startOffset;
  }

  bool IOStoreReader::readContainerHeader() {
    size_t cursor = 0;
    FIoContainerHeader containerHeader{.version = EIoContainerHeaderVersion::Initial};
    FIoChunkId chunkId{.id = m_TocHeader.containerId.id, .chunkIndex = 0, .chunkType = 6};

    auto buffer = readChunk(chunkId);
    if (buffer.size() < sizeof(FIoContainerHeader))
      return false;

    containerHeader.magic = readPrimitive<uint32_t>(buffer, cursor);
    if (containerHeader.magic != IO_CONTAINER_MAGIC) {
      Logger::logError("[IOStore]", "Wrong container magic. Found {:x}", containerHeader.magic);
      return false;
    }

    containerHeader.version = readPrimitive<EIoContainerHeaderVersion>(buffer, cursor);
    containerHeader.containerId = readStruct<FPackageId>(buffer, cursor);

    if (containerHeader.version < EIoContainerHeaderVersion::OptionalSegmentPackages) {
      (void) readPrimitive<uint32_t>(buffer, cursor); // skip
    }

    auto packageIdsCount = readPrimitive<uint32_t>(buffer, cursor);
    containerHeader.packageIds = readArray<FPackageId>(buffer, cursor, packageIdsCount);

    auto storeEntriesSize = readPrimitive<uint32_t>(buffer, cursor);
    auto storeEntriesEnd = storeEntriesSize + cursor;

    containerHeader.storeEntries.resize(packageIdsCount);
    for (uint32_t i = 0; i < packageIdsCount; ++i) {
      containerHeader.storeEntries[i] = readFilePackageStoreEntry(buffer, cursor, containerHeader.version);
    }

    cursor = storeEntriesEnd;

    if (containerHeader.version >= EIoContainerHeaderVersion::OptionalSegmentPackages) {
      auto optPackageIdsCount = readPrimitive<uint32_t>(buffer, cursor);
      containerHeader.optionalSegmentPackageIds = readArray<FPackageId>(buffer, cursor, optPackageIdsCount);

      auto optStoreEntriesSize = readPrimitive<uint32_t>(buffer, cursor);
      size_t optStoreEntriesEnd = cursor + optStoreEntriesSize;

      containerHeader.optionalSegmentStoreEntries.resize(optPackageIdsCount);
      for (uint32_t i = 0; i < optPackageIdsCount; ++i) {
        containerHeader.optionalSegmentStoreEntries[i] = readFilePackageStoreEntry
            (buffer, cursor, containerHeader.version);
      }

      cursor = optStoreEntriesEnd;
    }

    if (containerHeader.version >= EIoContainerHeaderVersion::Initial) {
      containerHeader.containerNameMap = loadNameBatch(buffer, cursor);
    }

    if (containerHeader.version >= EIoContainerHeaderVersion::LocalizedPackages) {
      auto localizedCount = readPrimitive<uint32_t>(buffer, cursor);
      containerHeader.localizedPackages = readArray<FIoContainerHeaderLocalizedPackage>(buffer, cursor, localizedCount);
    }

    {
      auto redirectsCount = readPrimitive<uint32_t>(buffer, cursor);
      containerHeader.packageRedirects = readArray<FIoContainerHeaderPackageRedirect>(buffer, cursor, redirectsCount);
    }

    this->m_containerHeader = std::move(containerHeader);
    return true;
  }

  FFilePackageStoreEntry IOStoreReader::readFilePackageStoreEntry
  (const std::vector<uint8_t> &buffer, size_t &cursor, EIoContainerHeaderVersion version) {
    FFilePackageStoreEntry entry{};
    if (version >= EIoContainerHeaderVersion::Initial) {
      if (version < EIoContainerHeaderVersion::NoExportInfo) {
        entry.exportCount = readPrimitive<uint32_t>(buffer, cursor);
        entry.exportBundleCount = readPrimitive<uint32_t>(buffer, cursor);
      }

      entry.importedPackage = readCArrayView<FPackageId>(buffer, cursor);
      entry.shaderMapHashes = readCArrayView<FIoHash>(buffer, cursor);
    } else {
      cursor += 8; // ExportBundlesSize (skip 8)
      entry.exportCount = readPrimitive<uint32_t>(buffer, cursor);
      entry.exportBundleCount = readPrimitive<uint32_t>(buffer, cursor);
      cursor += 8; // LoadOrder + Pad
      entry.importedPackage = readCArrayView<FPackageId>(buffer, cursor);
    }
    return entry;
  }


  FIoContainerHeaderSoftPackageReferences IOStoreReader::readSoftPackageReferences
  (const std::vector<uint8_t> &buffer, size_t &cursor) {
    FIoContainerHeaderSoftPackageReferences out;
    // The C# implementation:
    // bContainsSoftPackageReferences = Ar.ReadBoolean();
    // if (b) { PackageIds = Ar.ReadArray<FPackageId>(); PackageIndices = Ar.ReadArray<byte>(); }
    // We'll implement similarly:

    if (cursor + 1 > buffer.size()) throw std::runtime_error("readSoftPackageReferences out of range");
    out.bContainsSoftPackageReferences = buffer[cursor] != 0;
    cursor += 1;

    if (out.bContainsSoftPackageReferences) {
      // Read array of FPackageId (we assume the format is count + elements)
      // Our readArray requires explicit count, so we assume array is stored as "uint32 count, data..."
      // If actual format differs, adapt accordingly.
      auto pkgCount = readPrimitive<uint32_t>(buffer, cursor);
      out.PackageIds = readArray<FPackageId>(buffer, cursor, pkgCount);

      auto idxCount = readPrimitive<uint32_t>(buffer, cursor);
      if (idxCount) {
        if (cursor + idxCount > buffer.size())
          throw std::runtime_error
              ("readSoftPackageReferences indices out of range");
        out.PackageIndices.resize(idxCount);
        std::memcpy(out.PackageIndices.data(), buffer.data() + cursor, idxCount);
        cursor += idxCount;
      }
    }

    return out;
  }

  bool IOStoreReader::parseDirectoryPathList() {
    if (m_DirectoryIndexBuffer.empty()) {
      Logger::logWarn("[Path List]", "Buffer leer.");
      return false;
    }

    size_t cursor = 0;
    size_t bufferSize = m_DirectoryIndexBuffer.size();

    // 1. Mount Point lesen (FString)
    std::string mountPoint = readFString(m_DirectoryIndexBuffer, cursor);
    Logger::logInfo("[Path List]", "Mount Point: {}", mountPoint);

    // 2. Directory Entries (Array)
    if (cursor + sizeof(uint32_t) > bufferSize) { return false; }

    uint32_t dirEntryCount = 0;
    std::memcpy(&dirEntryCount, m_DirectoryIndexBuffer.data() + cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);

    size_t dirEntriesSize = dirEntryCount * sizeof(FIoDirectoryIndexEntry);
    if (cursor + dirEntriesSize > bufferSize) {
      Logger::logError("[Path List]", "Buffer Overflow bei Directory Entries.");
      return false;
    }
    cursor += dirEntriesSize; // Überspringen (wir wollen nur die Strings)

    // 3. File Entries (Array)
    if (cursor + sizeof(uint32_t) > bufferSize) { return false; }

    uint32_t fileEntryCount = 0;
    std::memcpy(&fileEntryCount, m_DirectoryIndexBuffer.data() + cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);

    size_t fileEntriesSize = fileEntryCount * sizeof(FIoFileIndexEntry);
    if (cursor + fileEntriesSize > bufferSize) {
      Logger::logError("[Path List]", "Buffer Overflow bei File Entries.");
      return false;
    }
    cursor += fileEntriesSize; // Überspringen

    // 4. String Table (Array von FStrings)
    if (cursor + sizeof(uint32_t) > bufferSize) { return false; }

    uint32_t stringCount = 0;
    std::memcpy(&stringCount, m_DirectoryIndexBuffer.data() + cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);

    Logger::logInfo("[Path List]", "Lese {} Pfad-Strings...", stringCount);

    m_DirectoryPaths.reserve(stringCount);
    for (uint32_t i = 0; i < stringCount; ++i) {
      std::string path = readFString(m_DirectoryIndexBuffer, cursor);
      m_DirectoryPaths.push_back(path);

      // Optional: Loggen der ersten paar Pfade zur Kontrolle
      if (i < 5) { Logger::logDebug("[Path List]", "String {}: {}", i, path); }
    }

    return true;
  }

  void IOStoreReader::createChunkLookupTable() {
    // Stellen Sie sicher, dass beide Vektoren die gleiche Größe haben
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

  bool IOStoreReader::resolveChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) {
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

  bool IOStoreReader::resolveImperfectChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) {
    auto iter = m_ImperfectChunkOffsetMap.find(chunkId);
    if (iter == m_ImperfectChunkOffsetMap.end()) {
      outOffsetLength = {};
      return false;
    }
    outOffsetLength = iter->second;
    return true;
  }

  std::vector<uint8_t> IOStoreReader::readChunk(const FIoChunkId &chunkId) {
    std::vector<uint8_t> uncompressedData;

    FIoOffsetAndLength offsetLenght{};

    if (!resolveChunk(chunkId, offsetLenght)) {
      Logger::logError("[Chunk Read]", "Chunk {} nicht gefunden.", chunkId.chunkIndex);
      return {};
    }

    uint32_t index = offsetLenght.offset / m_TocHeader.compressionBlockSize;
    const uint64_t lenght = offsetLenght.lenght;

    Logger::logError("[Chunk Read]", "Chunk {}.", chunkId.chunkIndex);

    uncompressedData.resize(lenght);
    for (uint64_t offset = 0; offset < lenght; index++) {
      auto &block = m_CompressionBlocks[index];
      auto dst = std::span(uncompressedData).subspan(offset, block.uncompressedSize);
      bool success = readBlock(block, dst);
      if (!success) {
        uncompressedData.clear();
        Logger::logError("[Chunk Read]", "Fehler beim Lesen oder Dekomprimieren von Block {}.", 0);
        return uncompressedData;
      }
      offset += block.uncompressedSize;
    }
    return uncompressedData;
  }

  bool IOStoreReader::readBlock(const FIoStoreTocCompressedBlockEntry &blockEntry, std::span<uint8_t> dst) {
    uint64_t compressedOffset = blockEntry.compressedOffset;
    uint32_t compressedSize = blockEntry.compressedSize;
    uint32_t uncompressedSize = blockEntry.uncompressedSize;
    uint8_t methodIndex = blockEntry.compressionMethod;

    CompressionMethod compressionMethod;
    if (methodIndex < m_CompressionMethods.size()) { compressionMethod = m_CompressionMethods[methodIndex]; } else {
      Logger::logError("[IOStore]", "Unbekannter Kompressionsindex {} gefunden.", methodIndex);
      return {};
    }

    std::vector<uint8_t> compressedData(compressedSize);
    if (!readFromUcasFile(compressedOffset, compressedSize, compressedData.data())) {
      Logger::logError("[IOStore]", "Fehler beim Lesen der komprimierten Daten aus der .ucas-Datei.");
      return {};
    }

    bool success = Decompressor::decompress(compressedData, dst, compressionMethod);

    return success;
  }

  bool IOStoreReader::readFromUcasFile(uint64_t offset, uint32_t size, uint8_t *buffer) {
    if (size == 0) {
      return true; // Nichts zu lesen, erfolgreich
    }

    std::ifstream ucasFile(m_UcasPath, std::ios::in | std::ios::binary);

    if (!ucasFile.is_open()) {
      Logger::logError("[UCAS Read]", "Konnte UCAS-Datei nicht öffnen: {}", m_UcasPath.string());
      return false;
    }
    try { ucasFile.seekg((long) offset, std::ios::beg); } catch (const std::exception &e) {
      Logger::logError("[UCAS Read]", "Fehler beim Springen (seekg) zu Offset {}: {}", offset, e.what());
      ucasFile.close();
      return false;
    }

    if (!ucasFile.good()) {
      Logger::logError
          (
           "[UCAS Read]", "Fehler nach seekg zu Offset {}. Möglicherweise außerhalb der Dateigröße.", offset);
      ucasFile.close();
      return false;
    }

    ucasFile.read(reinterpret_cast<char *>(buffer), size);

    if ((size_t) ucasFile.gcount() != size) {
      Logger::logError
          (
           "[UCAS Read]", "Lesefehler: {} Bytes angefordert, aber nur {} Bytes gelesen.", size, ucasFile.gcount());
      ucasFile.close();
      return false;
    }

    ucasFile.close();

    Logger::logDebug("[UCAS Read]", "Erfolgreich {} Bytes ab Offset {} aus UCAS-Datei gelesen.", size, offset);
    return true;
  }
} // namespace SAP
