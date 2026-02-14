//
// Created by oleub on 11.01.26.
//

#pragma once
#include "material_interface.hpp"

#include "assets/material/e_blend_mode.hpp"
#include "assets/material/e_material_shading_model.hpp"
#include "assets/material/e_translucency_lighting_mode.hpp"

#include "ue/object_factory.hpp"

namespace SAP {
  class Material : public MaterialInterface {
    friend class MaterialExporter;
    enum class EMaterialInterface : uint32_t {
      TwoSided = fnv1a("TwoSided"),
      bDisableDepthTest = fnv1a("bDisableDepthTest"),
      bIsMasked = fnv1a("bIsMasked"),
      Expressions = fnv1a("Expressions"),
      BlendMode = fnv1a("BlendMode"),
      TranslucencyLightingMode = fnv1a("TranslucencyLightingMode"),
      ShadingModel = fnv1a("ShadingModel"),
      OpacityMaskClipValue = fnv1a("OpacityMaskClipValue"),
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

  SAP_REGISTER_ASSET(Material)
} // namespace SAP
