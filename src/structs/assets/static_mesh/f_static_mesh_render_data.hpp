//
// Created by oleub on 07.01.26.
//

#pragma once
#include "f_static_mesh_LOD_resources.hpp"

#include "assets/nanite/f_nanit_resources.hpp"

#include "core/f_box_sphere_bounds.hpp"

namespace SAA {
  struct FStaticMeshRenderData {
    std::vector<FStaticMeshLODResources> LODResources;
    FNaniteResources naniteResources;
    FBoxSphereBounds bounds;
    std::vector<float> screenSize;
    bool bLODsShareStaticLighting;
  };
} 
