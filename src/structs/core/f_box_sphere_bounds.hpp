//
// Created by oleub on 07.01.26.
//

#pragma once
#include "math/f_vector.hpp"

namespace SAP {
#pragma pack(push, 1)
  struct FBoxSphereBounds {
    FVector origin;
    FVector extent;
    double sphereRadius;
  };
#pragma pack(pop)
} // namespace SAP
