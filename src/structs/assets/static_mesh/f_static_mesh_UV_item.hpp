//
// Created by oleub on 07.01.26.
//

#pragma once
#include <vector>

#include "assets/meshes/f_mesh_UV_float.hpp"

#include "render_core/f_packed_normal.hpp"

namespace SAA {
  struct FStaticMeshUVItem {
    std::array<FPackedNormal, 3> normals;
    std::span<const FMeshUVFloat> UVs;
  };
} 
