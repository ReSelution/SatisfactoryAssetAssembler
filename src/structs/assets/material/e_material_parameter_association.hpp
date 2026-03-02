//
// Created by oleub on 13.01.26.
//

#pragma once
#include <cstdint>

namespace SAA {
  enum class EMaterialParameterAssociation : uint32_t {
    LayerParameter = fnv1a("LayerParameter"),
    BlendParameter = fnv1a("BlendParameter"),
    GlobalParameter = fnv1a("GlobalParameter"),
  };
}
