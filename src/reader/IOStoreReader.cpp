#include "IOStoreReader.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "Decompressor.hpp"
#include "Reader.hpp"
#include "byte_cursor.hpp"
#include "core/constants.hpp"
#include "io/f_io_directory_index_enty.hpp"
#include "io/f_io_file_index_entry.hpp"
#include "io/f_on_demand_toc_entry.hpp"
#include "stats.hpp"

#include "vfs.hpp"

#include "game_file.hpp"
#include "sc/logger/logger.hpp"
#include "sc/memory/monotonic_resource.hpp"

using IOStoreLog = sc::Logger<"SAA", "IOSTORE">;

namespace SAA {
  IOStoreReader::IOStoreReader(std::filesystem::path utocPath, const std::filesystem::path &ucasPath) :
      Reader(Type::IO_STORE, std::move(utocPath)), m_ucasReader(ucasPath), m_utocReader(m_Path) {
    alocMutipilyer = 1.3;
  }

  bool IOStoreReader::load() {
    auto name    = m_Path.filename().string();
    auto t_total = IOStoreLog::time("Loaded {1} in {0}", name);

    Reader::load();

    ByteCursor cursor(m_utocReader.getSpan(0, m_utocReader.size()));

    parseHeader(cursor);
    parseChunkIds(cursor);
    parseOffsetLengths(cursor);
    // TODO: createChunkLookupTable() laeuft vor parsePerfectHashTable(), m_ChunkIndicesWithoutPerfectHash ist hier
    // also noch leer und m_ImperfectChunkOffsetMap bleibt immer leer. resolveImperfectChunk() findet dadurch nie
    // etwas. Die beiden Aufrufe muessten vertauscht werden. War vor der Portierung schon so.
    createChunkLookupTable();
    parsePerfectHashTable(cursor);
    parseCompressionBlocks(cursor);
    parseCompressionMethods(cursor);
    parseSignatures(cursor);
    parseDirectoryIndex(cursor);
    parseDirectoryPathList();
    parseTocMetas(cursor);

    if (m_Path.filename() != "global.utoc") {
      auto t = IOStoreLog::time("  Step 11: readContainerHeader: {0}");
      readContainerHeader();
    }

    return true;
  }

  bool IOStoreReader::mountVFS(Vfs *vfs) {
    if (m_DirectoryIndexBuffer.empty() || m_DirectoryPaths.empty()) {
      IOStoreLog::warn("Directory Index oder Pfade leer.");
      return false;
    }
    size_t entriesCount = 0;
    auto name = m_Path.filename().string();
    auto t    = IOStoreLog::time("{1}: Mounted {2} in {0}", name, entriesCount);


    ByteCursor cursor(m_DirectoryIndexBuffer);

    auto mountPoint = cursor.readFString(&m_storage);

    if (mountPoint.starts_with("../../..")) {
      mountPoint = mountPoint.substr(9); // Relativen Pfad entfernen
    }

    auto dirEntries = cursor.readArray<FIoDirectoryIndexEntry>();

    auto fileEntries = cursor.readArray<FIoFileIndexEntry>();

    vfs->reserveIOStore(fileEntries.size());
    entriesCount = fileEntries.size();


    // TODO: pathBuffer hat feste 512 Bytes, keiner der memcpy-Aufrufe unten prueft die Laenge. Ein tiefer Pfad oder
    // kaputte Indexdaten schreiben ueber den Stack-Puffer hinaus. Ebenso wird file.name anders als dir.name nicht
    // gegen m_DirectoryPaths.size() geprueft. War vor der Portierung schon so.
    char pathBuffer[512];
    char *const pathStart = pathBuffer;
    char *current         = pathBuffer;

    size_t mpSize = mountPoint.size();
    memcpy(current, mountPoint.data(), mpSize);
    current += mpSize;

    struct StackFrame {
      uint32_t dirIndex;
      char *pathPos; // Position im Buffer, an der dieses Verzeichnis startet
    };

    std::vector<StackFrame> stack;
    stack.reserve(64);

    if (!dirEntries.empty()) {
      stack.push_back({0, current});

      while (!stack.empty()) {
        StackFrame frame = stack.back();
        stack.pop_back();

        current = frame.pathPos;

        const auto &dir = dirEntries[frame.dirIndex];

        if (dir.name < m_DirectoryPaths.size()) {
          std::string_view dName = m_DirectoryPaths[dir.name];
          if (!dName.empty()) {
            // Slash hinzufügen, falls nötig
            if (current > pathStart && *(current - 1) != '/') {
              *current++ = '/';
            }
            memcpy(current, dName.data(), dName.size());
            current += dName.size();
          }
        }

        char *currentDirEnd = current;

        uint32_t fileIndex = dir.firstFileEntry;
        while (fileIndex != FIoDirectoryIndexResource_InvalidIndex) {
          const auto &file       = fileEntries[fileIndex];
          std::string_view fName = m_DirectoryPaths[file.name];

          char *filePointer = currentDirEnd;

          if (filePointer > pathStart && *(filePointer - 1) != '/') {
            *filePointer++ = '/';
          }

          memcpy(filePointer, fName.data(), fName.size());
          char *endOfPath = filePointer + fName.size();

          // String-View für den Mount-Aufruf erstellen (ohne neue Allokation)
          std::string_view fullPath(pathStart, endOfPath - pathStart);
          vfs->mountEntry(this, fullPath, m_ChunkIds[file.userData]);

          fileIndex = file.nextFileEntry;
          // current bleibt auf currentDirEnd, kein Reset nötig
        }

        // 3. Kinder auf den Stack legen
        uint32_t childIndex = dir.firstChildEntry;
        while (childIndex != FIoDirectoryIndexResource_InvalidIndex) {
          stack.push_back({childIndex, currentDirEnd});
          childIndex = dirEntries[childIndex].nextSiblingEntry;
        }
      }
    }

    return true;
  }

  void IOStoreReader::parseHeader(ByteCursor &cursor) {
    m_TocHeader = cursor.read<FIoStoreTocHeader>();
    if (memcmp(m_TocHeader.tocMagic.data(), EXPECTED_TOC_MAGIC.data(), EXPECTED_TOC_MAGIC.size()) != 0) {
      throw std::runtime_error("Wrong magic number.");
    }
    cursor = m_TocHeader.tocHeaderSize;
  }

  void IOStoreReader::parseChunkIds(ByteCursor &cursor) {
    m_ChunkIds = cursor.readArray<FIoChunkId>(m_TocHeader.tocEntryCount);
  }

  void IOStoreReader::parseOffsetLengths(ByteCursor &cursor) {
    m_ChunkOffsetLengths = cursor.readArray<FIoOffsetAndLength>(m_TocHeader.tocEntryCount);
  }

  bool operator>=(EIoStoreTocVersion lhs, uint8_t rhs) { return static_cast<uint8_t>(lhs) >= rhs; }

  void IOStoreReader::parsePerfectHashTable(ByteCursor &cursor) {
    m_ChunkPerfectHashSeeds          = cursor.readArray<int32_t>(m_TocHeader.tocChunkPerfectHashSeedsCount);
    m_ChunkIndicesWithoutPerfectHash = cursor.readArray<uint32_t>(m_TocHeader.tocChunksWithoutPerfectHashCount);
  }

  void IOStoreReader::parseCompressionBlocks(ByteCursor &cursor) {
    m_compressionBlocks = cursor.readArray<FIoStoreTocCompressedBlockEntry>(m_TocHeader.tocCompressedBlockEntryCount);
  }

  void IOStoreReader::parseCompressionMethods(ByteCursor &cursor) {
    uint32_t count  = m_TocHeader.compressionMethodNameCount;
    uint32_t length = m_TocHeader.compressionMethodNameLength;

    auto dataSize = count * length;
    auto buffer   = cursor.readArray<uint8_t>(dataSize);

    const uint8_t *methodBufferStart = buffer.data();
    m_CompressionMethods             = m_storage.allocateSpan<CompressionMethod>(count + 1);
    m_CompressionMethods[0]          = CompressionMethod::None;

    for (uint32_t i = 0; i < count; i++) {
      auto strStart               = i * length;
      std::string_view methodName = {reinterpret_cast<const char *>(methodBufferStart + strStart), length};
      size_t nullPos              = methodName.find('\0');
      if (nullPos != std::string::npos) {
        methodName = methodName.substr(0, nullPos);
      }
      CompressionMethod methodEnum = stringToCompressionMethod(methodName);
      m_CompressionMethods[i + 1]  = methodEnum;
    }
  }

  void IOStoreReader::parseSignatures(ByteCursor &cursor) const {
    if (!(m_TocHeader.containerFlags & EIoContainerFlags::SIGNED)) {
      return;
    }

    auto hashSize = cursor.read<int32_t>();

    auto tocAndBlockSignatureSize = hashSize * 2;
    auto chunkBlockSignatureSize  = FSHA_HASH_SIZE * m_TocHeader.tocCompressedBlockEntryCount;
    cursor += tocAndBlockSignatureSize + chunkBlockSignatureSize;
  }

  bool operator<(EIoStoreTocVersion lhs, uint32_t rhs) { return static_cast<uint8_t>(lhs) < rhs; }

  bool operator>(EIoStoreTocVersion lhs, uint32_t rhs) { return static_cast<uint8_t>(lhs) > rhs; }

  void IOStoreReader::parseDirectoryIndex(ByteCursor &cursor) {
    const uint32_t TOC_VERSION_DIRECTORY_INDEX     = 5;
    const EIoContainerFlags CONTAINER_FLAG_INDEXED = EIoContainerFlags::INDEXED;
    assert(m_TocHeader.version >= TOC_VERSION_DIRECTORY_INDEX && "TOC version is to Low");
    assert(((m_TocHeader.containerFlags & CONTAINER_FLAG_INDEXED) || m_TocHeader.directoryIndexSize == 0) &&
           "No Directory Index");

    m_DirectoryIndexBuffer = cursor.readArray<uint8_t>(m_TocHeader.directoryIndexSize);
  }

  void IOStoreReader::parseTocMetas(ByteCursor &cursor) const {
    uint32_t entryCount = m_TocHeader.tocEntryCount;

    auto totalHashSize = entryCount * FSHA_HASH_SIZE;

    cursor += totalHashSize;
    if (m_TocHeader.version >= TOC_VERSION_ONDEMAND_METADATA &&
        (m_TocHeader.containerFlags & EIoContainerFlags::ON_DEMAND)) {
      size_t entrySize         = sizeof(FOnDemandTocEntry);
      size_t totalOnDemandSize = (size_t) entryCount * entrySize;
      cursor += totalOnDemandSize;
    }
  }

  bool IOStoreReader::readContainerHeader() {
    FIoChunkId chunkId{.id = m_TocHeader.containerId.id, .chunkIndex = 0, .chunkType = 6};
    FIoContainerHeader containerHeader{
        .version = EIoContainerHeaderVersion::Initial,
    };

    auto file = readChunk(&m_storage, chunkId);
    assert(file != nullptr);
    containerHeader.magic = file->read<uint32_t>();
    if (containerHeader.magic != IO_CONTAINER_MAGIC) {
      IOStoreLog::err("Wrong container magic. Found {:x}", containerHeader.magic);
      return false;
    }

    containerHeader.version     = file->read<EIoContainerHeaderVersion>();
    containerHeader.containerId = file->read<FPackageId>();

    if (containerHeader.version < EIoContainerHeaderVersion::OptionalSegmentPackages) {
      *file += sizeof(uint32_t);
    }
    containerHeader.packageIds = file->readArray<FPackageId>();

    auto storeEntriesEnd = file->read<uint32_t>() + file->tell();

    containerHeader.storeEntriesById.reserve(containerHeader.packageIds.size());
    for (uint32_t i = 0; i < containerHeader.packageIds.size(); ++i) {
      const auto &[entry, insered] =
          containerHeader.storeEntriesById.emplace(containerHeader.packageIds[i], FFilePackageStoreEntry{});
      readFilePackageStoreEntry(file, &entry->second, containerHeader.version);
    }
    *file = storeEntriesEnd;

    if (containerHeader.version >= EIoContainerHeaderVersion::OptionalSegmentPackages) {
      containerHeader.optionalSegmentPackageIds = file->readArray<FPackageId>();

      auto optStoreEntriesEnd = file->read<uint32_t>() + file->tell();

      containerHeader.optionalSegmentStoreEntries =
          m_storage.allocateSpan<FFilePackageStoreEntry>(containerHeader.optionalSegmentPackageIds.size());
      for (uint32_t i = 0; i < containerHeader.optionalSegmentPackageIds.size(); ++i) {
        readFilePackageStoreEntry(file, &containerHeader.optionalSegmentStoreEntries[i], containerHeader.version);
      }

      *file = optStoreEntriesEnd;
    }

    if (containerHeader.version >= EIoContainerHeaderVersion::Initial) {
      containerHeader.containerNameMap = file->loadNameBatch(&m_storage);
    }

    if (containerHeader.version >= EIoContainerHeaderVersion::LocalizedPackages) {
      containerHeader.localizedPackages = file->readArray<FIoContainerHeaderLocalizedPackage>();
    }

    containerHeader.packageRedirects = file->readArray<FIoContainerHeaderPackageRedirect>();

    this->m_containerHeader = std::move(containerHeader);
    return true;
  }

  void IOStoreReader::readFilePackageStoreEntry(GameFile *file, FFilePackageStoreEntry *entry,
                                                EIoContainerHeaderVersion version) {
    if (version >= EIoContainerHeaderVersion::Initial) {
      if (version < EIoContainerHeaderVersion::NoExportInfo) {
        entry->exportCount       = file->read<uint32_t>();
        entry->exportBundleCount = file->read<uint32_t>();
      }

      entry->importedPackage = file->readArrayView<FPackageId>();
      entry->shaderMapHashes = file->readArrayView<FSHAHash>();
    } else {
      *file += 8; // ExportBundlesSize (skip 8)
      entry->exportCount       = file->read<uint32_t>();
      entry->exportBundleCount = file->read<uint32_t>();
      *file += 8; // LoadOrder + Pad
      entry->importedPackage = file->readArrayView<FPackageId>();
    }
  }

  //  FIoContainerHeaderSoftPackageReferences IOStoreReader::readSoftPackageReferences(ByteCursor &cursor) {
  //    FIoContainerHeaderSoftPackageReferences out;
  //
  //    out.bContainsSoftPackageReferences = file->readBool();
  //
  //    if (out.bContainsSoftPackageReferences) {
  //      out.PackageIds = file->readArray<FPackageId>();
  //      out.PackageIndices = file->readArray<uint8_t>();
  //    }
  //
  //    return out;
  //  }

  void IOStoreReader::parseDirectoryPathList() {
    if (m_DirectoryIndexBuffer.empty()) {
      return;
    }

    ByteCursor cursor(m_DirectoryIndexBuffer);
    (void) cursor.readFString(&m_storage); // skip the mount point

    auto dirEntryCount = cursor.read<uint32_t>();
    cursor += dirEntryCount * sizeof(FIoDirectoryIndexEntry); // Skip we only want the strings

    auto fileEntryCount = cursor.read<uint32_t>();

    cursor += fileEntryCount * sizeof(FIoFileIndexEntry); // Skip

    auto stringCount = cursor.read<uint32_t>();


    m_DirectoryPaths = m_storage.allocateSpan<std::string_view>(stringCount);
    for (uint32_t i = 0; i < stringCount; ++i) {
      m_DirectoryPaths[i] = cursor.readFString(&m_storage);
    }
  }

  void IOStoreReader::createChunkLookupTable() {
    m_ImperfectChunkOffsetMap.reserve(m_ChunkIndicesWithoutPerfectHash.size());
    for (const auto &i: m_ChunkIndicesWithoutPerfectHash) {
      m_ImperfectChunkOffsetMap.try_emplace(m_ChunkIds[i], m_ChunkOffsetLengths[i]);
    }
  }

  inline uint64_t hashChunk(uint32_t seed, const FIoChunkId *chunkId) {
    const auto *bytes         = reinterpret_cast<const uint8_t *>(chunkId);
    constexpr size_t datasize = sizeof(FIoChunkId);
    uint64_t hash             = seed != 0 ? static_cast<uint64_t>(seed) : 0xcbf29ce484222325;
    for (size_t i = 0; i < datasize; ++i) {
      hash = (hash * 0x00000100000001B3) ^ bytes[i];
    }
    return hash;
  }

  bool IOStoreReader::resolveChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const {
    if (m_TocHeader.tocEntryCount == 0) {
      outOffsetLength = {};
      return false;
    }

    // TODO: Ist die Seed-Tabelle leer, tocEntryCount aber nicht 0, wird unten durch 0 geteilt.
    auto seedCount = m_ChunkPerfectHashSeeds.size();
    auto seedIndex = hashChunk(0, &chunkId) % seedCount;
    auto seed      = m_ChunkPerfectHashSeeds[seedIndex];
    if (seed == 0) {
      outOffsetLength = {};
      return false;
    }
    uint32_t slot;
    if (seed < 0) {
      uint32_t seedAsIndex = -seed - 1;
      if (seedAsIndex < m_TocHeader.tocEntryCount) {
        slot = seedAsIndex;
      } else {
        return resolveImperfectChunk(chunkId, outOffsetLength);
      }
    } else {
      slot = hashChunk(seed, &chunkId) % m_TocHeader.tocEntryCount;
    }

    if (m_ChunkIds[slot] == chunkId) {
      outOffsetLength = m_ChunkOffsetLengths[slot];
      return true;
    }
    outOffsetLength = {};
    return false;
  }

  bool IOStoreReader::resolveImperfectChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const {
    auto iter = m_ImperfectChunkOffsetMap.find(chunkId);
    if (iter == m_ImperfectChunkOffsetMap.end()) {
      outOffsetLength = {};
      return false;
    }
    outOffsetLength = iter->second;
    return true;
  }

  GameFile *IOStoreReader::readChunk(sc::MonotonicResource *storage, const FIoChunkId &chunkId) const {
    FIoOffsetAndLength offsetLenght{};
    if (!resolveChunk(chunkId, offsetLenght)) {
      IOStoreLog::err("Chunk {} not found.", chunkId.chunkIndex);
      return {};
    }

    struct IOStoreFile : GameFileImpl {
      const IOStoreReader *reader;
      std::span<const FIoStoreTocCompressedBlockEntry> compressionBlocks;

      IOStoreFile(size_t fileSize, const IOStoreReader *reader,
                  std::span<const FIoStoreTocCompressedBlockEntry> blocks) :
          GameFileImpl(fileSize), reader(reader), compressionBlocks(blocks) {}

      uint64_t chunks() final { return compressionBlocks.size(); }

      bool load(size_t &index, std::span<uint8_t> dst) final {

        size_t offset     = chunkOffsetsLoaded[index].load(std::memory_order_relaxed) & VALUE_MASK;
        const auto &block = compressionBlocks[index];
        return reader->readBlock(block, dst.subspan(offset, block.uncompressedSize()));
      }
    };

    uint64_t index        = offsetLenght.offset() / m_TocHeader.compressionBlockSize;
    uint64_t chunks       = 0;
    const uint64_t lenght = offsetLenght.length();
    for (uint64_t offset = 0; offset < lenght; chunks++) {
      auto &block = m_compressionBlocks[index + chunks];
      auto size   = block.uncompressedSize();
      offset += size;
    }


    auto *file = storage->make<GameFile>(storage);
    auto impl  = file->createImpl<IOStoreFile>(offsetLenght.length(), this, m_compressionBlocks.subspan(index, chunks));

    size_t idx = 0;
    for (uint64_t offset = 0; offset < lenght; idx++) {
      auto &block                   = m_compressionBlocks[index + idx];
      auto size                     = block.uncompressedSize();
      impl->chunkOffsetsLoaded[idx] = offset;
      offset += size;
    }

    return file;
  }

  bool IOStoreReader::readBlock(const FIoStoreTocCompressedBlockEntry &blockEntry, std::span<uint8_t> dst) const {
    uint64_t compressedOffset = blockEntry.compressedOffset();
    uint32_t compressedSize   = blockEntry.compressedSize();
    uint8_t methodIndex       = blockEntry.compressionMethod;

    CompressionMethod compressionMethod;
    if (methodIndex < m_CompressionMethods.size()) {
      compressionMethod = m_CompressionMethods[methodIndex];
    } else {
      IOStoreLog::err("Unknown Compression methode: {}", methodIndex);
      return false;
    }

    std::span<const uint8_t> compressedData = m_ucasReader.getSpan(compressedOffset, compressedSize);
    DecompressedThroughput::record(dst.size());
    CompressedThroughput::record(compressedSize);
    return Decompressor::decompress(compressedData, dst, compressionMethod);
  }
} // namespace SAA
