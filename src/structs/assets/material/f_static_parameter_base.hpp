//
// Created by oleub on 13.01.26.
//

#pragma once
#include <string>

#include "f_material_parameter_info.hpp"

#include "core/f_guid.hpp"

namespace SAP {
  struct FStaticParameterBase {
    std::string name;
    FMaterialParameterInfo param;
    FGuid expressionGuid;
    bool override;
  };
}
