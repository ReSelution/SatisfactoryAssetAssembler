//
// Created by oleub on 11.01.26.
//

#pragma once

#include "f_material_texture_info.hpp"
#include "unreal_material.hpp"

#include "structs/gltf_material.hpp"

#include "sc/logger/logger.hpp"
#include "ue/textures/texture.hpp"

namespace SAA {
  using namespace sc::literals;

  using MatIntLog = sc::Logger<"SAA", "MaterialInterface">;
  class MaterialInterface : public UnrealMaterial {
    friend class MaterialExporter;

    enum class EMaterialInterfaceProperty : uint64_t {
      bUseMobileSpecular   = "bUseMobileSpecular"_h,
      MobileSpecularPower  = "MobileSpecularPower"_h,
      MobileSpecularMask   = "MobileSpecularMask"_h,
      FlattenedTexture     = "FlattenedTexture"_h,
      MobileBaseTexture    = "MobileBaseTexture"_h,
      MobileNormalTexture  = "MobileNormalTexture"_h,
      MobileMaskTexture    = "MobileMaskTexture"_h,
      TextureStreamingData = "TextureStreamingData"_h,
    };

  public:
    MaterialInterface(std::string_view classname = "MaterialInterface") : UnrealMaterial(classname) {};

    void exportToGltf(GltfMaterial &out) override {
      MatIntLog::err("Export to Gltf not implemented for {}", getClassName());
    };

  protected:
    void parse() override;

    std::weak_ptr<Texture> m_flattenedTexture;
    std::vector<FMaterialTextureInfo> m_textureStreamingData;
    std::optional<PropertyHolder> cachedExpressionData;

  private:
  };

  SAA_REGISTER_ASSET(MaterialInterface)
} // namespace SAA
