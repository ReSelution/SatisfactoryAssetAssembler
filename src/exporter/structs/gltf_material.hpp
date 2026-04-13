//
// Created by oleub on 13.01.26.
//

#pragma once
#include <string>

#include "exported_texture_data.hpp"

#include "ue/material/f_material_texture_info.hpp"

namespace SAA {
  struct GltfMaterial {
    std::string name;
    std::vector<ExportedTextureData> baseColors;
    std::vector<FMaterialTextureInfo> baseInfos;

    std::vector<ExportedTextureData> normals;
    std::vector<FMaterialTextureInfo> normalInfos;

    std::vector<ExportedTextureData> metallicRoughness;
    std::vector<FMaterialTextureInfo> rougInfos;

    float baseColorFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
  };
} 
