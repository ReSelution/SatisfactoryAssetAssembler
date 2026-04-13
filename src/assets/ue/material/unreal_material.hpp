//
// Created by oleub on 11.01.26.
//

#pragma once
#include "structs/gltf_material.hpp"

#include "asset_object.hpp"
#include "object_factory.hpp"

namespace SAA {
  class UnrealMaterial : public Object {
  public:
    UnrealMaterial(std::string_view classname = "UnrealMaterial") : Object(classname) {};
    virtual void exportToGltf(GltfMaterial &gltfMaterial);
  };

  SAA_REGISTER_ASSET(UnrealMaterial)
} 
