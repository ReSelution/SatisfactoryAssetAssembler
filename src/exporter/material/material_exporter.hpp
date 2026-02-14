//
// Created by oleub on 13.01.26.
//

#pragma once
#include "exporter.hpp"

#include "structs/gltf_material.hpp"

#include "ue/material/material.hpp"
#include "ue/material/material_instance.hpp"
#include "ue/material/material_instance_constant.hpp"

namespace SAP {
  class MaterialExporter : public Exporter {
  public:
    static GltfMaterial encodeMaterial(MaterialInterface *materialInterface);
    static void encodeMaterial(GltfMaterial &gltfMaterial, Material *material);
    static void encodeMaterial(GltfMaterial &gltfMaterial, MaterialInstance *material);
    static void encodeMaterial(GltfMaterial &gltfMaterial, MaterialInstanceConstant *material);

  private:
    static void addTexture(GltfMaterial &gltfMaterial, std::string_view textureType, ExportedTextureData &texData);
  };
} // namespace SAP
