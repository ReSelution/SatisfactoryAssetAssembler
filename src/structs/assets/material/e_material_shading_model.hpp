//
// Created by oleub on 11.01.26.
//

#pragma once

namespace SAA {
  using namespace sc::literals;

    enum class EMaterialShadingModel : uint64_t {
        MSM_Unlit = "MSM_Unlit"_h,
        MSM_DefaultLit = "MSM_DefaultLit"_h,
        MSM_Subsurface = "MSM_Subsurface"_h,
        MSM_PreintegratedSkin = "MSM_PreintegratedSkin"_h,
        MSM_ClearCoat = "MSM_ClearCoat"_h,
        MSM_SubsurfaceProfile = "MSM_SubsurfaceProfile"_h,
        MSM_TwoSidedFoliage = "MSM_TwoSidedFoliage"_h,
        MSM_Hair = "MSM_Hair"_h,
        MSM_Cloth = "MSM_Cloth"_h,
        MSM_Eye = "MSM_Eye"_h,
        MSM_SingleLayerWater = "MSM_SingleLayerWater"_h,
        MSM_ThinTranslucent = "MSM_ThinTranslucent"_h,
        MSM_Strata = "MSM_Strata"_h,
        MSM_NUM = "MSM_NUM"_h,
        MSM_FromMaterialExpression = "MSM_FromMaterialExpression"_h,
        MSM_MAX = "MSM_MAX"_h,
    };
}
