//
// Created by oleub on 09.01.26.
//

#pragma once

#include "byte_cursor.hpp"

#include "core/math/f_box.hpp"

#include "io/packages/f_byte_bulk_data.hpp"

namespace SAA {
  constexpr size_t NUM_MIPS = 3;
#pragma pack(push, 1)
  struct FSparseDistanceFieldMip {
    FIntVector indirectionDimensions;
    uint32_t numDistanceFieldBricks;
    FVector volumeToVirtualUVScale;
    FVector volumeToVirtualUVAdd;
    FVector2D distanceFieldToVolumeScaleBias;
    uint32_t bulkOffset;
    uint32_t bulkSize;
  };
#pragma pack(pop)
  struct FDistanceFieldVolumeData5 {
    FBox localSpaceMeshbounds;
    bool mostlyTwoSided;
    std::span<const FSparseDistanceFieldMip> mips;
    std::span<const uint8_t> alwaysLoadedMip;
    FByteBulkData streamableMips;

    static FDistanceFieldVolumeData5 parse(ByteCursor &cursor, AssetFileParser *parser) {
      return {
          .localSpaceMeshbounds = cursor.read<FBox>(),
          .mostlyTwoSided = cursor.readBool(),
          .mips = cursor.readArray<FSparseDistanceFieldMip>(NUM_MIPS),
          .alwaysLoadedMip = cursor.readArray<uint8_t>(),
          .streamableMips = parser->parseFByteBulkData(cursor),
      };
    }
  };
} 
