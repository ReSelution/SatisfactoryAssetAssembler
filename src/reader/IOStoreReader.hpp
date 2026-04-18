#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

#include "hash/hash.hpp"
#include <ankerl/unordered_dense.h>

#include "byte_cursor.hpp"
#include "Decompressor.hpp"
#include "Reader.hpp"
#include "file_reader.hpp"
#include "io/f_io_chunk_id.hpp"
#include "io/f_io_container_header.hpp"
#include "io/f_io_container_header_soft_package_refenrenes.hpp"
#include "io/f_io_offset_and_lenght.hpp"
#include "io/f_io_store_toc_Compressed_block_entry.hpp"
#include "io/f_io_store_toc_header.hpp"
#include "game_file.hpp"

namespace SAA {
  class IOStoreReader : public Reader {
  public:
    IOStoreReader(std::filesystem::path utocPath, const std::filesystem::path &ucasPath);

    IOStoreReader(const IOStoreReader &) = delete;

    IOStoreReader(IOStoreReader &&) = delete;

    IOStoreReader &operator=(const IOStoreReader &) = delete;

    IOStoreReader &operator=(IOStoreReader &&) = delete;

    bool load() override;

    GameFile *readChunk(SC::ChaosBumpArena *storage, const FIoChunkId &chunkId) const;

    bool mountVFS(Vfs *vfs) override;

    const FIoContainerHeader *containerHeader() const { return &m_containerHeader; }

  private:

    void parseHeader(ByteCursor &cursor);

    void parseChunkIds(ByteCursor &cursor);

    void parseOffsetLengths(ByteCursor &cursor);

    void parsePerfectHashTable(ByteCursor &cursor);

    void parseCompressionBlocks(ByteCursor &cursor);

    void parseCompressionMethods(ByteCursor &cursor);

    void parseSignatures(ByteCursor &cursor) const;

    void parseDirectoryIndex(ByteCursor &cursor);

    void parseTocMetas(ByteCursor &cursor) const;

    bool readContainerHeader();

    static void
    readFilePackageStoreEntry(GameFile *file, FFilePackageStoreEntry *entry, EIoContainerHeaderVersion version);

    void parseDirectoryPathList();

    void createChunkLookupTable();

    bool resolveChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const;

    bool resolveImperfectChunk(const FIoChunkId &chunkId, FIoOffsetAndLength &outOffsetLength) const;

    bool readBlock(const FIoStoreTocCompressedBlockEntry &blockEntry, std::span<uint8_t> dst) const;

    FileReader m_ucasReader;
    FileReader m_utocReader;

    FIoStoreTocHeader m_TocHeader{};
    std::span<const FIoStoreTocCompressedBlockEntry> m_compressionBlocks;
    std::span<CompressionMethod> m_CompressionMethods;
    std::span<const uint8_t> m_DirectoryIndexBuffer;
    std::span<std::string_view> m_DirectoryPaths;

    std::span<const int32_t> m_ChunkPerfectHashSeeds;
    std::span<const uint32_t> m_ChunkIndicesWithoutPerfectHash;

    std::span<const FIoChunkId> m_ChunkIds;
    std::span<const FIoOffsetAndLength> m_ChunkOffsetLengths;

    using denseMap = ankerl::unordered_dense::map<FIoChunkId, FIoOffsetAndLength>;

    denseMap m_ImperfectChunkOffsetMap;


    FIoContainerHeader m_containerHeader{};

  };
} 
