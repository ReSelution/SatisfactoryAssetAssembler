//
// Created by oleub on 11.01.26.
//

#pragma once
#include "material_interface.hpp"

#include "assets/material/e_blend_mode.hpp"
#include "assets/material/e_material_shading_model.hpp"
#include "assets/material/e_translucency_lighting_mode.hpp"

#include "object_factory.hpp"

namespace SAA {
  class Material : public MaterialInterface {
    friend class MaterialExporter;
      enum class EMaterialInterface : uint64_t {
          TwoSided                 = "TwoSided"_h,
          bDisableDepthTest        = "bDisableDepthTest"_h,
          bIsMasked                = "bIsMasked"_h,
          Expressions              = "Expressions"_h,
          BlendMode                = "BlendMode"_h,
          TranslucencyLightingMode = "TranslucencyLightingMode"_h,
          ShadingModel             = "ShadingModel"_h,
          OpacityMaskClipValue     = "OpacityMaskClipValue"_h,
      };

  public:
    Material(std::string_view classname = "Material") : MaterialInterface(classname) {};
    void exportToGltf(GltfMaterial &outMaterial) override;

  protected:
    void parse() override;
    bool twoSided{};
    bool disableDepthTest{};
    bool isMasked{};
    std::vector<std::weak_ptr<Object>> m_expressions;
    EBlendMode blendMode = EBlendMode::BLEND_Opaque;
    ETranslucencyLightingMode translucencyLightingMode = ETranslucencyLightingMode::TLM_VolumetricNonDirectional;
    EMaterialShadingModel shadingModel = EMaterialShadingModel::MSM_Unlit;
    float opacityMaskClipValue = 0.333f;
    std::vector<std::weak_ptr<Texture>> m_textures;

  private:
  };

  SAA_REGISTER_ASSET(Material)
} 
