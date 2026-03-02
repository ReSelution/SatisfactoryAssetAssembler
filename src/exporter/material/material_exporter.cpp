//
// Created by oleub on 13.01.26.
//

#include "material_exporter.hpp"

#include <map>

#include "texture/texture_exporter.hpp"

#include "ue/material/material_instance_constant.hpp"

namespace SAA {
  void MaterialExporter::encodeMaterial(GltfMaterial &gltfMaterial, Material *material) {
    for (auto &tex: material->m_textures) {
      auto texture = tex.lock().get();
      texture->safeParse();
      ExportedTextureData texData = TextureExporter::encodeAsset(texture, EImageMimeType::PNG);
      addTexture(gltfMaterial, texture->objName(), texData);
    }
  }

  void MaterialExporter::encodeMaterial(GltfMaterial &gltfMaterial, MaterialInstance *material) {
    Logger::logInfo("MaterialExporter", "MaterialInstance encodeing not implementet");
  }

  void MaterialExporter::encodeMaterial(GltfMaterial &gltfMaterial, MaterialInstanceConstant *material) {
    for (auto &texPrams: material->texture_parameters) {
      auto texture = texPrams.texture.lock().get();
      texture->safeParse();
      ExportedTextureData texData = TextureExporter::encodeAsset(texture, EImageMimeType::PNG);
      addTexture(gltfMaterial, texPrams.parameterInfo.name.text(), texData);
    }
    if (material->m_parent.lock()) {
      material->m_parent.lock()->safeParse();
      material->m_parent.lock()->exportToGltf(gltfMaterial);
    }
  }

  GltfMaterial MaterialExporter::encodeMaterial(MaterialInterface *materialInterface) {
    GltfMaterial gltfMaterial{.name = std::string(materialInterface->objName())};
    materialInterface->exportToGltf(gltfMaterial);
    return gltfMaterial;
  }

  void MaterialExporter::addTexture(GltfMaterial &gltfMaterial, std::string_view textureType,
                                    ExportedTextureData &texData) {
    std::string type = std::string(textureType);

    if (type.find("_Alb") != std::string::npos || type.find("Albedo") != std::string::npos ||
        type.find("_BC") != std::string::npos)
    // type.find("flipbook") != std::string::npos ||
    // type.find("Diffuse") != std::string::npos)
    {
      gltfMaterial.baseColors.emplace_back(std::move(texData));
    } else if (type.find("_Nor") != std::string::npos || type.find("_N") != std::string::npos) {
      gltfMaterial.normals.emplace_back(std::move(texData));
    } else if (type.find("_Rough") != std::string::npos || type.find("Mask") != std::string::npos ||
               type.find("_MREO") != std::string::npos ||
               type.find("_Forms") != std::string::npos) { // "Forms" ist UE5 Standard bei Satisfactory
      gltfMaterial.metallicRoughness.emplace_back(std::move(texData));
    } else if (type.find("Noise") != std::string::npos || type.find("Weather") != std::string::npos ||
               type.find("Debug") != std::string::npos) {
      Logger::logInfo("[MaterialExporter]", "Skipping auxiliary texture: {}", type);
    } else {
      Logger::logError("[MaterialExporter]", "Unknown texture group: {} -> {} ", textureType, texData.name);
    }
  }
} // namespace SAP
