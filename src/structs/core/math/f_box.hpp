//
// Created by oleub on 06.01.26.
//

#pragma once
#include <cstdint>

#include "f_vector.hpp"

namespace SAP {
#pragma pack(push, 1)
  struct FBox {
    FVector min;
    FVector max;
    uint8_t isValid;
  };

  struct FBox_f {
    FVector_f min;
    FVector_f max;
    uint8_t isValid;
  };
#pragma pack(pop)
} // namespace SAP
