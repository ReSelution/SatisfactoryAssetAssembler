//
// Created by oleub on 24.01.26.
//

#pragma once
#include <memory>

#include "core/name.hpp"

namespace SAP {
  class Object;

  struct FMaterialOverrideNanite {
    FName overrideMaterialRef;
    bool enabled;
    Object* overrideMaterial;
  };
} // namespace SAP
