//
// Created by oleub on 20.12.25.
//

#pragma once
#include <array>
#include <cstdint>

#include "f_io_toc_container_id.hpp"
#include "core/f_guid.hpp"

namespace SAP {

  static const std::array<uint8_t, 16> EXPECTED_TOC_MAGIC = {
    0x2D, 0x3D, 0x3D, 0x2D, 0x2D, 0x3D, 0x3D, 0x2D, 0x2D, 0x3D, 0x3D, 0x2D, 0x2D, 0x3D, 0x3D, 0x2D
  }; // -==--==--==--==-


  enum class EIoStoreTocVersion : uint8_t {
    INVALID                            = 0,
    INITIAL                            = 1,
    DIRECTORY_INDEX                    = 2,
    PARTITION_SIZE                     = 3,
    PERFECT_HASH                       = 4,
    PERFECT_HASH_WITH_OVERFLOW         = 5,
    ON_DEMAND_META_DATA                = 6,
    REMOVED_ON_DEMAND_META_DATA        = 7,
    REPLACE_IO_CHUNK_HASH_WITH_IO_HASH = 8,
    LATEST_PLUS_ONE                    = 9,
    LATEST                             = LATEST_PLUS_ONE - 1
  };

  enum class EIoContainerFlags : uint8_t {
    NONE       = 0,
    COMPRESSED = (1 << 0), // 1
    ENCRYPTED  = (1 << 1), // 2
    SIGNED     = (1 << 2), // 4
    INDEXED    = (1 << 3), // 8
    ON_DEMAND  = (1 << 4), // 16
  };

  inline EIoContainerFlags operator|(EIoContainerFlags lhs, EIoContainerFlags rhs) {
    return static_cast<EIoContainerFlags>(static_cast<std::underlying_type_t<EIoContainerFlags>>(lhs) |
                                          static_cast<std::underlying_type_t<EIoContainerFlags>>(rhs));
  }

  inline bool operator&(EIoContainerFlags lhs, EIoContainerFlags rhs) {
    return (static_cast<std::underlying_type_t<EIoContainerFlags>>(lhs) &
            static_cast<std::underlying_type_t<EIoContainerFlags>>(rhs)) != 0;
  }

#pragma pack(push)


  struct FIoStoreTocHeader {
    std::array<uint8_t, 16> tocMagic;
    EIoStoreTocVersion version;
    uint8_t reserved0;
    uint16_t reserved1;

    uint32_t tocHeaderSize;
    uint32_t tocEntryCount;
    uint32_t tocCompressedBlockEntryCount;
    uint32_t tocCompressedBlockEntrySize;

    uint32_t compressionMethodNameCount;
    uint32_t compressionMethodNameLength;

    uint32_t compressionBlockSize;
    uint32_t directoryIndexSize;

    uint32_t partitionCount;

    FIoTocContainerId containerId;
    FGuid encryptionKeyGuid;

    EIoContainerFlags containerFlags;
    uint8_t reserved3;
    uint16_t reserved4;

    uint32_t tocChunkPerfectHashSeedsCount;
    uint64_t partitionSize;
    uint32_t tocChunksWithoutPerfectHashCount;

    uint32_t reserved7;
    std::array<uint64_t, 5> reserved8;
  };


#pragma pack(pop)
}
