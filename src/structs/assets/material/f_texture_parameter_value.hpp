//
// Created by oleub on 14.01.26.
//

#pragma once
#include "f_material_parameter_info.hpp"

#include "core/name.hpp"

#include "ue/textures/texture.hpp"

namespace SAA {
  struct FTextureParameterValue {
    FName mame;
    FMaterialParameterInfo parameterInfo;
    std::weak_ptr<Texture> texture;
    FGuid guid;

    // static FTextureParameterValue get(FPropertyValue *val) {
    //   FTextureParameterValue ret{};
    //   if (auto *fallback = std::get_if<FStructFallback>(&val->Data)) {
    //     ret.mame = fallback->getOrDefault<FName>(fnv1a("ParameterName"), []() { return FName{}; });
    //     ret.guid = fallback->getOrDefault<FGuid>(fnv1a("ExpressionGUID"), []() { return FGuid{}; });
    //     ret.parameterInfo = fallback->getStructAs<FMaterialParameterInfo>
    //         (fnv1a("ParameterInfo"), [](FStructFallback *fallback) { return FMaterialParameterInfo::get(fallback); },
    //          []() { return FMaterialParameterInfo{}; });
    //     ret.texture = fallback->getObjectAs<Texture>
    //         (fnv1a("ParameterValue"), []() { return std::weak_ptr<Texture>(); });
    //   }
    //   return ret;
    // }
  };
} // namespace SAP
