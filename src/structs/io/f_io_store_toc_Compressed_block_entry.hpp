//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push, 1)
  struct FIoStoreTocCompressedBlockEntry_raw {
    std::array<uint8_t, 5> compressedOffsetArray; // 5 Bytes
    std::array<uint8_t, 3> compressedSizeArray;   // 3 Bytes
    std::array<uint8_t, 3> uncompressedSizeArray; // 3 Bytes
    uint8_t compressionMethod;                    // 1 Byte

    [[nodiscard]] constexpr uint64_t compressedOffset() const {
      return static_cast<uint64_t>(compressedOffsetArray[0]) |
             (static_cast<uint64_t>(compressedOffsetArray[1]) << 8) |
             (static_cast<uint64_t>(compressedOffsetArray[2]) << 16) |
             (static_cast<uint64_t>(compressedOffsetArray[3]) << 24) |
             (static_cast<uint64_t>(compressedOffsetArray[4]) << 32);
    }

    [[nodiscard]] constexpr uint32_t compressedSize() const {
      return static_cast<uint32_t>(compressedSizeArray[0]) |
             (static_cast<uint32_t>(compressedSizeArray[1]) << 8) |
             (static_cast<uint32_t>(compressedSizeArray[2]) << 16);
    }

    [[nodiscard]] constexpr uint32_t uncompressedSize() const {
      return static_cast<uint32_t>(uncompressedSizeArray[0]) |
             (static_cast<uint32_t>(uncompressedSizeArray[1]) << 8) |
             (static_cast<uint32_t>(uncompressedSizeArray[2]) << 16);
    }
  };
#pragma pack(pop)
  struct FIoStoreTocCompressedBlockEntry {
    uint64_t compressedOffset;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint8_t compressionMethod;

    static FIoStoreTocCompressedBlockEntry get(const FIoStoreTocCompressedBlockEntry_raw &raw) {
      return FIoStoreTocCompressedBlockEntry{
        .compressedOffset = raw.compressedOffset(),
        .compressedSize = raw.compressedSize(),
        .uncompressedSize = raw.uncompressedSize(),
        .compressionMethod = raw.compressionMethod
      };
    }
  };

}
