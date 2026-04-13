//
// Created by oleub on 11.01.26.
//

#pragma once
#include <cstdint>

namespace SAA {
    enum class EBlendMode : uint64_t {
        BLEND_Opaque         = "BLEND_Opaque"_h,
        BLEND_Masked         = "BLEND_Masked"_h,
        BLEND_Translucent    = "BLEND_Translucent"_h,
        BLEND_Additive       = "BLEND_Additive"_h,
        BLEND_Modulate       = "BLEND_Modulate"_h,
        BLEND_AlphaComposite = "BLEND_AlphaComposite"_h,
        BLEND_AlphaHoldout   = "BLEND_AlphaHoldout"_h,
        BLEND_MAX            = "BLEND_MAX"_h,
    };
}
