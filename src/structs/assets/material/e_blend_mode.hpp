//
// Created by oleub on 11.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  enum class EBlendMode : uint32_t {
    BLEND_Opaque = fnv1a("BLEND_Opaque"),
    BLEND_Masked = fnv1a("BLEND_Masked"),
    BLEND_Translucent = fnv1a("BLEND_Translucent"),
    BLEND_Additive = fnv1a("BLEND_Additive"),
    BLEND_Modulate = fnv1a("BLEND_Modulate"),
    BLEND_AlphaComposite = fnv1a("BLEND_AlphaComposite"),
    BLEND_AlphaHoldout = fnv1a("BLEND_AlphaHoldout"),
    BLEND_MAX = fnv1a("BLEND_MAX"),
  };
}
