#include "PAKReader.hpp"

#include "Reader.hpp"
#include "ThroughputStats.hpp"
#include "byte_cursor.hpp"
#include "vfs.hpp"

#include "logger/chaos_logger.hpp"
LOG_ALIAS(PakLog, "SAA", "PAK")

namespace SAA {
  PAKReader::PAKReader(const std::filesystem::path &path) : Reader(Reader::Type::PAK, path), m_pakFile(path) {
  }

  bool PAKReader::load() {
    auto name = m_Path.filename().string();
    auto t = PakLog::time("Loaded {1} in {0}", name );
    readPaKInfo();
    return true;
  }

  void PAKReader::readPaKInfo() {
    // Size 8a
    auto offset = m_pakFile.size() - std::min(m_pakFile.size(), static_cast<size_t>(221));
    auto length = m_pakFile.size() - offset;
    ByteCursor cursor(m_pakFile.getSpan(offset, length));
    m_pakInfo.encryptionGuid = cursor.read<FGuid>();
    m_pakInfo.encryptionIndex = cursor.read<uint8_t>() != 0;
    m_pakInfo.magic = cursor.read<uint32_t>();
    if (m_pakInfo.magic != PAK_FILE_MAGIC) {
      throw std::runtime_error("PAK file magic doesn't match");
    }
    m_pakInfo.version = cursor.read<EPakFileVersion>();
    m_pakInfo.indexOffset = cursor.read<int64_t>();
    m_pakInfo.indexSize = cursor.read<int64_t>();
    m_pakInfo.hash = cursor.read<FSHAHash>();
    m_pakInfo.compressionMethods = readCompressionMethods(cursor);
  }

  std::vector<CompressionMethod> PAKReader::readCompressionMethods(ByteCursor &cursor) {
    constexpr uint32_t MAX_NUM_COMRESSION_METHODE = 5;
    constexpr uint32_t COMPRESSION_METHOD_NAME_LEN = 32;
    constexpr uint32_t dataSize = MAX_NUM_COMRESSION_METHODE * COMPRESSION_METHOD_NAME_LEN;

    std::vector<CompressionMethod> compressionMethods;
    compressionMethods.reserve(MAX_NUM_COMRESSION_METHODE + 1);
    compressionMethods.emplace_back(CompressionMethod::None);
    auto buffer = cursor.readArray<uint8_t>(dataSize);
    const uint8_t *methodBufferStart = buffer.data();
    for (uint32_t i = 0; i < MAX_NUM_COMRESSION_METHODE; i++) {
      auto strStart = i * COMPRESSION_METHOD_NAME_LEN;
      std::string methodName =
          (std::string(reinterpret_cast<const char *>(methodBufferStart + strStart), COMPRESSION_METHOD_NAME_LEN));
      size_t nullPos = methodName.find('\0');
      if (nullPos != std::string::npos) {
        methodName.resize(nullPos);
      }
      if (methodName.empty()) {
        break;
      }
      CompressionMethod methodEnum = stringToCompressionMethod(methodName);
      compressionMethods.emplace_back(methodEnum);
    }
    return compressionMethods;
  }

  uint32_t PAKReader::readPakEntry(ByteCursor &cursor, uint32_t offset) {
    cursor = offset;
    FPakEntry entry;

    auto bitField = cursor.read<uint32_t>();
    auto blockSize = (bitField & 0x3f) == 0x3f ? cursor.read<uint32_t>() : (bitField & 0x3f) << 11;
    auto compressionIndex = ((bitField >> 23) & 0x3f);
    entry.compressionMethod = m_pakInfo.compressionMethods[compressionIndex];

    auto isOffset32BitSafe = static_cast<bool>(bitField & (1 << 31));
    entry.offset = isOffset32BitSafe ? cursor.read<uint32_t>() : cursor.read<uint64_t>();

    auto isUncompressedSize32BitSafe = static_cast<bool>(bitField & (1 << 30));
    entry.uncompressedSize = isUncompressedSize32BitSafe ? cursor.read<uint32_t>() : cursor.read<uint64_t>();

    if (entry.compressionMethod != CompressionMethod::None) {
      auto isSize32BitSafe = static_cast<bool>(bitField & (1 << 29));
      entry.compressedSize = isSize32BitSafe ? cursor.read<uint32_t>() : cursor.read<uint64_t>();
    } else {
      entry.compressedSize = entry.uncompressedSize;
    }

    auto compressionBlockCount = (bitField >> 6) & 0xffff;
    entry.compressedBlocks.reserve(compressionBlockCount);
    if (compressionBlockCount == 1) {
      entry.compressionBlockSize = entry.uncompressedSize;
    } else {
      entry.compressionBlockSize = blockSize;
    }

    auto structSize = sizeof(uint64_t) * 3 + sizeof(uint32_t) * 2 + 1 + 20;
    if (entry.compressionMethod != CompressionMethod::None) {
      structSize += sizeof(uint32_t) + compressionBlockCount * 2 * sizeof(uint64_t);
    }
    entry.offset += structSize;
    auto blockOffset = entry.offset;
    if (compressionBlockCount == 1) {
      entry.compressedBlocks.emplace_back(blockOffset, entry.compressedSize);
    } else {
      for (uint32_t i = 0; i < compressionBlockCount; i++) {
        auto &block = entry.compressedBlocks.emplace_back(blockOffset, cursor.read<uint32_t>());
        blockOffset += block.compressedSize;
      }
    }
    m_pakEntries.emplace_back(std::move(entry));
    return m_pakEntries.size() - 1;
  }

  bool PAKReader::mountVFS(Vfs *vfs) {
    int32_t fileCount;
    auto name = m_Path.filename().string();
    auto t = PakLog::time("{1}: Mounted {2} in {0}", name, fileCount);

    ByteCursor index(m_pakFile.getSpan(m_pakInfo.indexOffset, m_pakInfo.indexSize));
    index.setAbsoluteOffset(m_pakInfo.indexOffset);
    auto mountPoint = index.readFString(stringStorage.get());
    if (mountPoint.starts_with("../../..")) {
      mountPoint = mountPoint.substr(9); // Relativen Pfad entfernen
    }

    fileCount = index.read<int32_t>();
    vfs->reservePak(fileCount);
    m_pakEntries.reserve(fileCount);
    index += 8; // PathHashSeed
    // PathHashIndexOffset (long) + PathHashIndexSize (long) + PathHashIndexHash (20 bytes)
    if (index.readBool())
      index += 36;

    if (!index.readBool()) {
      return false;
    }
    auto dirIndexOffset = index.read<int64_t>();
    auto dirIndexSize = index.read<int64_t>();
    index += FSHA_HASH_SIZE;

    auto encodedPakEntrySize = index.read<uint32_t>();
    auto encodedPakEntries = index.subspan(index.tell(), encodedPakEntrySize);
    index += encodedPakEntries.size();

    auto filesNum = index.read<int32_t>();
    assert(filesNum >= 0 && "NonEncodedEntries not Implented");

    ByteCursor dirIndex(m_pakFile.getSpan(dirIndexOffset, dirIndexSize));
    auto dirIndexCount = dirIndex.read<uint32_t>();

    std::string pathBuffer;
    pathBuffer.reserve(512); //
    pathBuffer = mountPoint;

    for (uint32_t i = 0; i < dirIndexCount; i++) {
      size_t oldLen = pathBuffer.size();

      pathBuffer += dirIndex.readFString(stringStorage.get());
      auto fileEntries = dirIndex.read<uint32_t>();
      for (uint32_t j = 0; j < fileEntries; j++) {
        size_t dirLen = pathBuffer.size();
        pathBuffer += dirIndex.readFString(stringStorage.get());

        auto offset = dirIndex.read<uint32_t>();
        auto fileIndex = readPakEntry(encodedPakEntries, offset);
        vfs->mountEntry(this, pathBuffer, fileIndex);
        pathBuffer.resize(dirLen);
      }
      pathBuffer.resize(oldLen);
    }

    return true;
  }

  std::span<uint8_t> PAKReader::readFile(HeapStorage *storage, uint32_t index) const {
    auto &entry = m_pakEntries[index];

    std::span<uint8_t> uncompressedData = storage->allocateSpan(entry.uncompressedSize);
    auto uncompressedSpan = std::span(uncompressedData);
    if (entry.compressionMethod == CompressionMethod::None) {
      auto src = m_pakFile.getSpan(entry.offset, entry.uncompressedSize);
      Decompressor::decompress(src, uncompressedData, entry.compressionMethod);
      return uncompressedData;
    }

    for (uint64_t blockIndex = 0, offset = 0; offset < entry.uncompressedSize;
         blockIndex++, offset += entry.compressionBlockSize) {
      auto &block = entry.compressedBlocks[blockIndex];
      auto src = m_pakFile.getSpan(block.compressedOffset, block.compressedSize);

      auto uncompressedBlockSize = std::min((uint64_t) entry.compressionBlockSize, entry.uncompressedSize - offset);
      auto dst = uncompressedSpan.subspan(offset, uncompressedBlockSize);

      bool succes = Decompressor::decompress(src, dst, entry.compressionMethod);
      if (!succes) {
        PakLog::err("Error decompressing entry {}.", index);
        return {};
      }
    }
    ThroughputStats::totalDecompressedBytes.fetch_add(uncompressedData.size(), std::memory_order_relaxed);
    ThroughputStats::totalCompressedBytes.fetch_add(entry.compressedSize, std::memory_order_relaxed);
    return uncompressedData;
  }
} // namespace SAP
