//
// Created by oleub on 11.01.26.
//

#pragma once

namespace SAP {
  enum class EMaterialShadingModel : uint32_t {
    MSM_Unlit = fnv1a("MSM_Unlit"),
    MSM_DefaultLit = fnv1a("MSM_DefaultLit"),
    MSM_Subsurface = fnv1a("MSM_Subsurface"),
    MSM_PreintegratedSkin = fnv1a("MSM_PreintegratedSkin"),
    MSM_ClearCoat = fnv1a("MSM_ClearCoat"),
    MSM_SubsurfaceProfile = fnv1a("MSM_SubsurfaceProfile"),
    MSM_TwoSidedFoliage = fnv1a("MSM_TwoSidedFoliage"),
    MSM_Hair = fnv1a("MSM_Hair"),
    MSM_Cloth = fnv1a("MSM_Cloth"),
    MSM_Eye = fnv1a("MSM_Eye"),
    MSM_SingleLayerWater = fnv1a("MSM_SingleLayerWater"),
    MSM_ThinTranslucent = fnv1a("MSM_ThinTranslucent"),
    MSM_Strata = fnv1a("MSM_Strata"),
    MSM_NUM = fnv1a("MSM_NUM"),
    MSM_FromMaterialExpression = fnv1a("MSM_FromMaterialExpression"),
    MSM_MAX = fnv1a("MSM_MAX")
  };
}
