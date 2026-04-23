//
// Created by oleub on 07.01.26.
//

#pragma once
#include "math/f_vector.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FBoxSphereBounds {
    FVector origin;
    FVector extent;
    double sphereRadius;
  };

  struct FBoxSphereBoundsF {
    FVector3F origin;
    FVector3F extent;
    float sphereRadius;
  };
#pragma pack(pop)
} 
