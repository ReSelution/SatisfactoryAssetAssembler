//
// Created by oleub on 11.01.26.
//

#pragma once
#include "structs/gltf_material.hpp"

#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class UnrealMaterial : public Object {
  public:
    UnrealMaterial(std::string_view classname = "UnrealMaterial") : Object(classname) {};
    virtual void exportToGltf(GltfMaterial &gltfMaterial);
  };

  SAP_REGISTER_ASSET(UnrealMaterial)
} // SAP
