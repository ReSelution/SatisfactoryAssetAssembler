//
// Created by oleub on 14.01.26.
//

#pragma once
#include "f_material_parameter_info.hpp"

#include "core/f_guid.hpp"
#include "core/name.hpp"

#include "ue/properties/f_property_tag.hpp"

namespace SAP {
  struct FScalarParameterValue {
    FName parameterName;
    float value;
    FMaterialParameterInfo paramInfo;
    FGuid expressionGuid;

    static FScalarParameterValue get(FPropertyValue *val) {
      FScalarParameterValue ret{};
      // if (auto *strct = std::get_if<FStructFallback>(&val->Data)) {
      //   ret.parameterName = strct->getOrDefault<FName>(fnv1a("ParameterName"), []() { return FName(); });
      //   ret.value = strct->getOrDefault<float>(fnv1a("ParameterValue"), []() { return 0.0f; });
      //   ret.paramInfo = strct->getStructAs<FMaterialParameterInfo>
      //       (fnv1a("ParameterInfo"), [](FStructFallback *fallback) { return FMaterialParameterInfo::get(fallback); },
      //        []() { return FMaterialParameterInfo(); });
      //   ret.expressionGuid = strct->getOrDefault<FGuid>(fnv1a("ExpressionGUID"), []() { return FGuid(); });
      // }
       return ret;
    }
  };
}
