//
// Created by oleub on 04.02.26.
//

#pragma once
#include <cstdint>

#include "Decompressor.hpp"

namespace SAP {
  struct FPakCompressedBlock {
    uint64_t compressedOffset;
    uint64_t compressedSize;
  };

  struct FPakEntry {
    uint64_t offset;
    uint64_t compressedSize;
    uint64_t uncompressedSize;
    CompressionMethod compressionMethod;
    std::vector<FPakCompressedBlock> compressedBlocks;
    uint32_t compressionBlockSize;
  };
} // namespace SAP
