//
// Created by oleub on 16.01.26.
//

#pragma once
#include "f_material_parameter_info.hpp"

#include "core/f_guid.hpp"
#include "core/f_linear_color.hpp"
#include "core/name.hpp"

#include "ue/properties/f_property_tag.hpp"

namespace SAA {
  struct FVectorParameterValue {
    FName name;
    FMaterialParameterInfo parameterInfo;
    FLinearColor color;
    FGuid guid;
    //
    // static FVectorParameterValue get(FPropertyValue *val) {
    //   if (auto *fallback  = std::get_if<FStructFallback>(&val->Data)) {
    //     return FVectorParameterValue{
    //       fallback->getOrDefault<FName>(fnv1a("ParameterName"), [](){return FName();}),
    //       };
    //   }
    //   return FVectorParameterValue{};
    // }
  };
} 
