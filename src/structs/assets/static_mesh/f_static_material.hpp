//
// Created by oleub on 09.01.26.
//

#pragma once
#include <memory>

#include "assets/meshes/f_mesh_UV_channel_info.hpp"

#include "ue/asset_object.hpp"
#include "ue/material/material_interface.hpp"

namespace SAP {
  struct FStaticMaterial {
    MaterialInterface* materialInterface;
    FName materialSlotName;
    FName importedMaterialSlotName;
    FMeshUVChannelInfo UVChannelInfo;
    std::shared_ptr<SAP::Object> overlayMaterialInterface;
  };

} // namespace SAP
