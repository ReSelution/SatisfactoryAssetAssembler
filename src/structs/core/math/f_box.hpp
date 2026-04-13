//
// Created by oleub on 06.01.26.
//

#pragma once
#include <cstdint>

#include "f_vector.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FBox {
    FVector min;
    FVector max;
    uint8_t isValid;
  };

  struct FBox_f {
    FVector3F min;
    FVector3F max;
    uint8_t isValid;
  };

  struct FBox2f {
    FVector2F min;
    FVector2F max;
    uint8_t isValid;
  };
  struct FBox3f {
    FVector3F min;
    FVector3F max;
    uint8_t isValid;
  };
#pragma pack(pop)
} 
