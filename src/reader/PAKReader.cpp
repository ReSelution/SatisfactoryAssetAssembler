#include "PAKReader.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>

#include "Decompressor.hpp"
#include "Reader.hpp"
#include "byte_cursor.hpp"
#include "stats.hpp"
#include "vfs.hpp"

#include "sc/logger/logger.hpp"
#include "sc/memory/monotonic_resource.hpp"

using PakLog = sc::Logger<"SAA", "PAK">;

namespace SAA {
  PAKReader::PAKReader(const std::filesystem::path &path)
      : Reader(Reader::Type::PAK, path), m_pakFile(path) {}

  bool PAKReader::load() {
    auto name = m_Path.filename().string();
    auto t = PakLog::time("Loaded {1} in {0}", name);
    readPaKInfo();
    return true;
  }

  void PAKReader::readPaKInfo() {
    // Size 8a
    auto offset =
        m_pakFile.size() - std::min(m_pakFile.size(), static_cast<size_t>(221));
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

  std::vector<CompressionMethod>
  PAKReader::readCompressionMethods(ByteCursor &cursor) {
    constexpr uint32_t MAX_NUM_COMRESSION_METHODE = 5;
    constexpr uint32_t COMPRESSION_METHOD_NAME_LEN = 32;
    constexpr uint32_t dataSize =
        MAX_NUM_COMRESSION_METHODE * COMPRESSION_METHOD_NAME_LEN;

    std::vector<CompressionMethod> compressionMethods;
    compressionMethods.reserve(MAX_NUM_COMRESSION_METHODE + 1);
    compressionMethods.emplace_back(CompressionMethod::None);
    auto buffer = cursor.readArray<uint8_t>(dataSize);
    const uint8_t *methodBufferStart = buffer.data();
    for (uint32_t i = 0; i < MAX_NUM_COMRESSION_METHODE; i++) {
      auto strStart = i * COMPRESSION_METHOD_NAME_LEN;
      std::string methodName = (std::string(
          reinterpret_cast<const char *>(methodBufferStart + strStart),
          COMPRESSION_METHOD_NAME_LEN));
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
    auto blockSize = (bitField & 0x3f) == 0x3f ? cursor.read<uint32_t>()
                                               : (bitField & 0x3f) << 11;
    auto compressionIndex = ((bitField >> 23) & 0x3f);
    // TODO: compressionIndex (bis 63) wird nicht gegen compressionMethods.size() (hoechstens 6) geprueft.
    entry.compressionMethod = m_pakInfo.compressionMethods[compressionIndex];

    auto isOffset32BitSafe = static_cast<bool>(bitField & (1 << 31));
    entry.offset =
        isOffset32BitSafe ? cursor.read<uint32_t>() : cursor.read<uint64_t>();

    auto isUncompressedSize32BitSafe = static_cast<bool>(bitField & (1 << 30));
    entry.uncompressedSize = isUncompressedSize32BitSafe
                             ? cursor.read<uint32_t>()
                             : cursor.read<uint64_t>();

    if (entry.compressionMethod != CompressionMethod::None) {
      auto isSize32BitSafe = static_cast<bool>(bitField & (1 << 29));
      entry.compressedSize =
          isSize32BitSafe ? cursor.read<uint32_t>() : cursor.read<uint64_t>();
    }
    else {
      entry.compressedSize = entry.uncompressedSize;
    }

    auto compressionBlockCount = (bitField >> 6) & 0xffff;
    entry.compressedBlocks.reserve(compressionBlockCount);
    if (compressionBlockCount == 1) {
      entry.compressionBlockSize = entry.uncompressedSize;
    }
    else {
      entry.compressionBlockSize = blockSize;
    }

    auto structSize = sizeof(uint64_t) * 3 + sizeof(uint32_t) * 2 + 1 + 20;
    if (entry.compressionMethod != CompressionMethod::None) {
      structSize +=
          sizeof(uint32_t) + compressionBlockCount * 2 * sizeof(uint64_t);
    }
    entry.offset += structSize;
    auto blockOffset = entry.offset;
    if (compressionBlockCount == 1) {
      entry.compressedBlocks.emplace_back(blockOffset, entry.compressedSize);
    }
    else {
      for (uint32_t i = 0; i < compressionBlockCount; i++) {
        auto &block = entry.compressedBlocks.emplace_back(
            blockOffset, cursor.read<uint32_t>());
        blockOffset += block.compressedSize;
      }
    }
    m_pakEntries.emplace_back(std::move(entry));
    return m_pakEntries.size() - 1;
  }

  bool PAKReader::mountVFS(Vfs *vfs) {
    int32_t fileCount = 0;
    auto name = m_Path.filename().string();

    ByteCursor index(
        m_pakFile.getSpan(m_pakInfo.indexOffset, m_pakInfo.indexSize));
    index.setAbsoluteOffset(m_pakInfo.indexOffset);

    auto mountPoint = index.readFString(&m_storage);
    if (mountPoint.starts_with("../../..")) {
      mountPoint = mountPoint.substr(9);
    }

    fileCount = index.read<int32_t>();
    vfs->reservePak(fileCount);
    m_pakEntries.reserve(fileCount);

    index += 8; // PathHashSeed
    if (index.readBool())
      index += 36; // PathHashIndex Daten überspringen

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
    assert(filesNum >= 0 && "NonEncodedEntries not Implemented");

    ByteCursor dirIndex(m_pakFile.getSpan(dirIndexOffset, dirIndexSize));
    auto dirIndexCount = dirIndex.read<uint32_t>();

    // Vorbereitung des Pfad-Buffers
    // TODO: pathBuffer hat feste 512 Bytes, keiner der memcpy-Aufrufe prueft die Laenge. Ein langer Pfad oder
    // kaputte Indexdaten schreiben ueber den Stack-Puffer hinaus. War vor der Portierung schon so.
    char pathBuffer[512];
    char *const pathStart = pathBuffer;

    // Mountpoint kopieren
    memcpy(pathBuffer, mountPoint.data(), mountPoint.size());
    char *pos = pathBuffer + mountPoint.size();

    // Log-Timer starten (jetzt wo fileCount bekannt ist)
    auto t = PakLog::time("{1}: Mounted {2} entries in {0}", name, fileCount,
                          mountPoint);

    for (uint32_t i = 0; i < dirIndexCount; i++) {
      char *dirBasePos = pos;

      auto dirName = dirIndex.readFString(&m_storage);
      if (!dirName.empty()) {
        // Slash hinzufügen, falls nötig
        if (pos > pathStart && *(pos - 1) != '/') {
          *pos++ = '/';
        }
        memcpy(pos, dirName.data(), dirName.size());
        pos += dirName.size();
      }

      char *pathWithDirPos = pos;

      auto fileEntriesCount = dirIndex.read<uint32_t>();
      for (uint32_t j = 0; j < fileEntriesCount; j++) {
        auto fileName = dirIndex.readFString(&m_storage);
        auto entryOffset = dirIndex.read<uint32_t>();

        char *filePos = pathWithDirPos;
        if (filePos > pathStart && *(filePos - 1) != '/') {
          *filePos++ = '/';
        }

        memcpy(filePos, fileName.data(), fileName.size());
        char *endOfPath = filePos + fileName.size();

        // VFS Mount
        auto fileIndex = readPakEntry(encodedPakEntries, entryOffset);
        vfs->mountEntry(this,
                        std::string_view{pathStart, static_cast<size_t>(
                            endOfPath - pathStart)},
                        fileIndex);
      }

      pos = dirBasePos;
    }

    return true;
  }

  GameFile *PAKReader::readFile(sc::MonotonicResource *storage, uint32_t entryIndex) const {
    auto &entry = m_pakEntries[entryIndex];

    struct PAKFileImpl : GameFileImpl {
      const PAKReader *reader;
      const FPakEntry &entry;

      PAKFileImpl(const PAKReader *r, const FPakEntry &e)
          : GameFileImpl(e.uncompressedSize), reader(r), entry(e) {}

      uint64_t chunks() final {
        return entry.compressionMethod == CompressionMethod::None
               ? 1
               : entry.compressedBlocks.size();
      }

      bool load(size_t &index, std::span<uint8_t> dst) final {

        size_t offset = chunkOffsetsLoaded[index].load(std::memory_order_relaxed) & VALUE_MASK;
        bool success = false;
        std::span<const uint8_t> src;
        size_t decompressedSize = 0;
        if (entry.compressionMethod == CompressionMethod::None) {
          src = reader->m_pakFile.getSpan(entry.offset, entry.uncompressedSize);
          decompressedSize =entry.uncompressedSize;
          success = Decompressor::decompress(src, dst.subspan(offset,decompressedSize), entry.compressionMethod);
        }
        else {
          auto &block = entry.compressedBlocks[index];
          src = reader->m_pakFile.getSpan(block.compressedOffset, block.compressedSize);
         decompressedSize = std::min<size_t>(entry.compressionBlockSize, entry.uncompressedSize - offset);
          success = Decompressor::decompress(src, dst.subspan(offset, decompressedSize), entry.compressionMethod);
        }

        if (success) [[likely]] {
          DecompressedThroughput::record(decompressedSize);
          CompressedThroughput::record(src.size());
        }
        return success;
      }
    };
    auto *file = storage->make<GameFile>(storage);
    auto impl = file->createImpl<PAKFileImpl>(this, entry);

    size_t offset = 0;

    for (size_t i = 0; i < impl->chunks(); i++) {
      impl->chunkOffsetsLoaded[i] = offset;
      offset += entry.compressionBlockSize;
    }
    return file;
  }

} // namespace SAA
