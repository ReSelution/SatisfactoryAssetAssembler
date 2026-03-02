//
// Created by oleub on 06.01.26.
//

#pragma once
#include <cstdint>

namespace SAA {
#pragma pack(push, 1)
  struct FVector_f {
    float x, y, z;
  };

  struct FVector4d {
    double x, y, z, w;
  };

  struct FVector4_f {
    float x, y, z, w;
  };

  struct FVector {
    double x, y, z;
  };

  struct FVector2F {
    float x, y;
  };

  struct FVector2D {
    double x, y;
  };

  struct FIntVector {
    int32_t x, y, z;
  };

  struct FIntVector2D {
    int32_t x, y;
  };

  struct FRotator {
    double x, y, z;
  };
#pragma pack(pop)

  static_assert(sizeof(FVector) == 24 && "Sizeof FVector Wrong");
} // namespace SAP
