//
// Created by oleub on 20.12.25.
//

#pragma once
#include "f_io_chunk_id.hpp"
#include "f_io_hash.hpp"

namespace SAA {
#pragma pack(push, 1)

  struct FOnDemandTocEntry {
    FSHAHash Hash;                        // 20 Bytes
    FIoChunkId ChunkId{};                 // 12 Bytes
    uint64_t RawSize{};                   // 8 Bytes
    uint64_t EncodedSize{};               // 8 Bytes
    uint32_t FirstCompressedBlockIndex{}; // 4 Bytes (BlockOffset)
    uint32_t CompressedBlockNum{};        // 4 Bytes (BlockCount)
  };

#pragma pack(pop)
} // namespace SAP
