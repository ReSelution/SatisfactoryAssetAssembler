#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <unordered_map>
#include <vector>

#include "../util/byte_cursor.hpp"
#include "Decompressor.hpp"
#include "file_reader.hpp"
#include "Reader.hpp"
#include "io/f_io_chunk_id.hpp"
#include "io/f_io_container_header.hpp"
#include "io/f_io_container_header_soft_package_refenrenes.hpp"
#include "io/f_io_offset_and_lenght.hpp"
#include "io/f_io_store_toc_Compressed_block_entry.hpp"
#include "io/f_io_store_toc_header.hpp"

namespace SAP {
  class IOStoreReader : public Reader {
  public:
    IOStoreReader(std::filesystem::path utocPath, const std::filesystem::path &ucasPath);

    IOStoreReader(const IOStoreReader &) = delete;
    IOStoreReader(IOStoreReader &&) = delete;
    IOStoreReader &operator=(const IOStoreReader &) = delete;
    IOStoreReader &operator=(IOStoreReader &&) = delete;

    bool load() override;
    std::vector<uint8_t> readChunk(const FIoChunkId &chunkId) const;
    bool mountVFS(Vfs *vfs) override;

    const FIoContainerHeader *containerHeader() const { return &m_containerHeader; }

  private:
    std::vector<uint8_t> utocBuffer;
    void parseHeader(ByteCursor &cursor);
    void parseChunkIds(ByteCursor &cursor);
    void parseOffsetLengths(ByteCursor &cursor);
    void parsePerfectHashTable(ByteCursor &cursor);
    void parseCompressionBlocks(ByteCursor &cursor);
    void parseCompressionMethods(ByteCursor &cursor);
    void parseSignatures(ByteCursor &cursor);
    void parseDirectoryIndex(ByteCursor &cursor);
    void parseTocMetas(ByteCursor &cursor);

    bool readContainerHeader();
    static FFilePackageStoreEntry readFilePackageStoreEntry
    (ByteCursor &cursor, EIoContainerHeaderVersion version);

    static FIoContainerHeaderSoftPackageReferences readSoftPackageReferences
    (ByteCursor &cursor);
    void parseDirectoryPathList();
    void createChunkLookupTable();

    bool resolveChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const ;
    bool resolveImperfectChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const ;

    bool readBlock(const FIoStoreTocCompressedBlockEntry &blockEntry, std::span<uint8_t> dst) const ;

    FileReader m_ucasReader;

    FIoStoreTocHeader m_TocHeader;
    std::vector<FIoStoreTocCompressedBlockEntry> m_CompressionBlocks;
    std::vector<CompressionMethod> m_CompressionMethods = {CompressionMethod::None};
    std::span<const uint8_t> m_DirectoryIndexBuffer;
    std::vector<std::string_view> m_DirectoryPaths;

    std::span<const int32_t> m_ChunkPerfectHashSeeds;
    std::span<const uint32_t> m_ChunkIndicesWithoutPerfectHash;

    std::span<const FIoChunkId> m_ChunkIds;
    std::vector<FIoOffsetAndLength> m_ChunkOffsetLengths;

    std::unordered_map<FIoChunkId, FIoOffsetAndLength> m_ImperfectChunkOffsetMap;

    FIoContainerHeader m_containerHeader{};

    HeapStorage<> m_stringStorage{};
  };
} // namespace SAP
