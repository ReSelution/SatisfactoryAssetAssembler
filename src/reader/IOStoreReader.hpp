#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <unordered_map>
#include <vector>

#include "Decompressor.hpp"
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
    IOStoreReader(std::filesystem::path utocPath, std::filesystem::path ucasPath);
    ~IOStoreReader();

    IOStoreReader(const IOStoreReader &) = delete;
    IOStoreReader(IOStoreReader &&) = delete;
    IOStoreReader &operator=(const IOStoreReader &) = delete;
    IOStoreReader &operator=(IOStoreReader &&) = delete;

    bool load() override;
    std::vector<uint8_t> readChunk(const FIoChunkId &chunkId) override;
    bool mountVFS(Vfs &vfs) override;

    const FIoContainerHeader& containerHeader() {return m_containerHeader;}

  private:
    size_t parseHeader(size_t offset);
    size_t parseChunkIds(size_t offset);
    size_t parseOffsetLengths(size_t offset);
    size_t parsePerfectHashTable(size_t offset);
    size_t parseCompressionBlocks(size_t offset);
    size_t parseCompressionMethods(size_t offset);
    size_t parseSignatures(size_t offset);
    size_t parseDirectoryIndex(size_t offset);
    size_t parseTocMetas(size_t offset);

    bool readContainerHeader();
    static FFilePackageStoreEntry readFilePackageStoreEntry
    (const std::vector<uint8_t> &buffer, size_t &cursor, EIoContainerHeaderVersion version);

    static FIoContainerHeaderSoftPackageReferences readSoftPackageReferences
    (const std::vector<uint8_t> &buffer, size_t &cursor);
    bool parseDirectoryPathList();
    void createChunkLookupTable();

    bool resolveChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength);
    bool resolveImperfectChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength);

    bool readBlock(const FIoStoreTocCompressedBlockEntry &blockEntry, std::span<uint8_t> dst);
    bool readFromUcasFile(uint64_t offset, uint32_t size, uint8_t *buffer);

    std::filesystem::path m_UcasPath;

    FIoStoreTocHeader m_TocHeader = {};
    std::vector<FIoStoreTocCompressedBlockEntry> m_CompressionBlocks;
    std::vector<CompressionMethod> m_CompressionMethods = {CompressionMethod::None};
    std::vector<uint8_t> m_DirectoryIndexBuffer;
    std::vector<std::string> m_DirectoryPaths;

    std::vector<int32_t> m_ChunkPerfectHashSeeds;
    std::vector<uint32_t> m_ChunkIndicesWithoutPerfectHash;

    std::vector<FIoChunkId> m_ChunkIds;
    std::vector<FIoOffsetAndLength> m_ChunkOffsetLengths;

    std::unordered_map<FIoChunkId, FIoOffsetAndLength> m_ImperfectChunkOffsetMap;

    FIoContainerHeader m_containerHeader;
  };
} // namespace SAP
