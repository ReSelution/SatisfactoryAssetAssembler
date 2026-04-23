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
  struct FSparseDistanceFieldMip4 {
    FIntVector indirectionDimensions;
    uint32_t numDistanceFieldBricks;
    FVector3F volumeToVirtualUVScale;
    FVector3F volumeToVirtualUVAdd;
    FVector2F distanceFieldToVolumeScaleBias;
    uint32_t bulkOffset;
    uint32_t bulkSize;

    // Standard-Konstruktor
    FSparseDistanceFieldMip4() = default;
  };

  struct FSparseDistanceFieldMip {
    FIntVector indirectionDimensions;
    uint32_t numDistanceFieldBricks;
    FVector volumeToVirtualUVScale;
    FVector volumeToVirtualUVAdd;
    FVector2D distanceFieldToVolumeScaleBias;
    uint32_t bulkOffset;
    uint32_t bulkSize;

    FSparseDistanceFieldMip() = default;

    // 1. Konstruktor: Erstellen aus Mip4 (Float -> Double)
    FSparseDistanceFieldMip(const FSparseDistanceFieldMip4& other)
        : indirectionDimensions(other.indirectionDimensions),
          numDistanceFieldBricks(other.numDistanceFieldBricks),
          volumeToVirtualUVScale(FVector::From(glm::vec3(other.volumeToVirtualUVScale))), // GLM macht float -> double automatisch
          volumeToVirtualUVAdd(FVector::From(glm::vec3(other.volumeToVirtualUVAdd))),
          distanceFieldToVolumeScaleBias(FVector2D::From(glm::vec2(other.distanceFieldToVolumeScaleBias))),
          bulkOffset(other.bulkOffset),
          bulkSize(other.bulkSize)
    {}

    // 2. Zuweisungsoperator: Mip = Mip4
    FSparseDistanceFieldMip& operator=(const FSparseDistanceFieldMip4& other) {
      indirectionDimensions = other.indirectionDimensions;
      numDistanceFieldBricks = other.numDistanceFieldBricks;
      volumeToVirtualUVScale = FVector::From(glm::vec3(other.volumeToVirtualUVScale));
      volumeToVirtualUVAdd = FVector::From(glm::vec3(other.volumeToVirtualUVAdd));
      distanceFieldToVolumeScaleBias = FVector2D::From(glm::vec2(other.distanceFieldToVolumeScaleBias));
      bulkOffset = other.bulkOffset;
      bulkSize = other.bulkSize;
      return *this;
    }
  };
#pragma pack(pop)
  struct FDistanceFieldVolumeData5 {
    FBox localSpaceMeshbounds;
    bool mostlyTwoSided;
    std::span<const FSparseDistanceFieldMip> mips;
    std::span<const uint8_t> alwaysLoadedMip;
    FByteBulkData *streamableMips;

    static FDistanceFieldVolumeData5 parse5_3(ByteCursor &cursor, AssetFileParser *parser) {
      return {
          .localSpaceMeshbounds = cursor.read<FBox>(),
          .mostlyTwoSided = cursor.readBool(),
          .mips = cursor.readArray<FSparseDistanceFieldMip>(NUM_MIPS),
          .alwaysLoadedMip = cursor.readArray<uint8_t>(),
          .streamableMips = parser->parseFByteBulkData(cursor),
      };
    }
    static FDistanceFieldVolumeData5 parse5_6(ByteCursor &cursor, AssetFileParser *parser) {
      auto box = cursor.read<FBox_f>();
      auto mostlyTwoSided = cursor.readBool();
      auto mipsC = cursor.readArray<FSparseDistanceFieldMip4>(NUM_MIPS);
      auto mips = cursor.m_owner->storage()->allocateSpan<FSparseDistanceFieldMip>(NUM_MIPS);
      for (uint32_t i = 0; i < NUM_MIPS; ++i) {
        mips[i] = mipsC[i];
      }
      return {
        .localSpaceMeshbounds = {box.min.x, box.min.y, box.min.z, box.max.x, box.max.y, box.max.z, box.isValid},
        .mostlyTwoSided = mostlyTwoSided,
        .mips = mips,
        .alwaysLoadedMip = cursor.readArray<uint8_t>(),
        .streamableMips = parser->parseFByteBulkData(cursor),
    };
    }
  };
} 
