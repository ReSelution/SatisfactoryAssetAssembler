//
// Created by oleub on 13.01.26.
//

#pragma once
#include "e_blend_mode.hpp"
#include "e_material_shading_model.hpp"

namespace SAA {
  struct FMaterialInstanceBasePropertyOverrides {
    EBlendMode blendMode;
    EMaterialShadingModel shadingModel;
    float opacityMaskClipValue;
    bool ditheredLODTransition;

    // static FMaterialInstanceBasePropertyOverrides parse(FStructFallback *fallback) {
    //   return FMaterialInstanceBasePropertyOverrides{
    //     .blendMode = fallback->getOrDefaultEnum<EBlendMode>(fnv1a("BlendMode"), []() { return EBlendMode{}; }),
    //     .shadingModel = fallback->getOrDefaultEnum<EMaterialShadingModel>(fnv1a("ShadingModel"), []() { return
    //     EMaterialShadingModel{}; }), .opacityMaskClipValue =
    //     fallback->getOrDefault<float>(fnv1a("opacityMaskClipValue"), [](){return 0.0f;}), .ditheredLODTransition =
    //     fallback->getOrDefault<bool>(fnv1a("DitheredLODTransition"), [](){return false;}),
    //   };
    // }
  };
} // namespace SAP
