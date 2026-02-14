//
// Created by oleub on 11.01.26.
//

#pragma once
#include "f_material_texture_info.hpp"
#include "unreal_material.hpp"

#include "structs/gltf_material.hpp"

#include "ue/textures/texture.hpp"

namespace SAP {
  class MaterialInterface : public UnrealMaterial {
    friend class MaterialExporter;

    enum class EMaterialInterfaceProperty : uint32_t {
      bUseMobileSpecular = fnv1a("bUseMobileSpecular"),
      MobileSpecularPower = fnv1a("MobileSpecularPower"),
      MobileSpecularMask = fnv1a("MobileSpecularMask"),
      FlattenedTexture = fnv1a("FlattenedTexture"),
      MobileBaseTexture = fnv1a("MobileBaseTexture"),
      MobileNormalTexture = fnv1a("MobileNormalTexture"),
      MobileMaskTexture = fnv1a("MobileMaskTexture"),
      TextureStreamingData = fnv1a("TextureStreamingData")
    };

  public:
    MaterialInterface(std::string_view classname = "MaterialInterface") : UnrealMaterial(classname) {};

    void exportToGltf(GltfMaterial &out) override {
      Logger::logError("[GLTF]", "Export to Gltf not implentet for {}", getClassName());
    };

  protected:
    void parse() override;
    std::weak_ptr<Texture> m_flattenedTexture;
    std::vector<FMaterialTextureInfo> m_textureStreamingData;
    std::optional<PropertyHolder> cachedExpressionData;

  private:
  };

  SAP_REGISTER_ASSET(MaterialInterface)
} // namespace SAP
