//
// Created by oleub on 13.01.26.
//

#pragma once
#include <vector>

#include "f_static_component_mask_parameter.hpp"
#include "f_static_material_layers_parameter.hpp"
#include "f_static_switch_parameter.hpp"
#include "f_static_terrain_layer_weight_parameter.hpp"

namespace SAP {
  struct FStaticParameterSet {
    std::vector<FStaticSwitchParameter> staticSwitchParameters;
    std::vector<FStaticComponentMaskParameter> staticComponentMaskParameters;
    std::vector<FStaticTerrainLayerWeightParameter> terrainLayerWeightParameters;
    std::vector<FStaticMaterialLayersParameter> materialLayersParameters;
    //
    // static FStaticParameterSet getStaticParameterSet(FStructFallback *fallback) {
    //   return FStaticParameterSet{
    //     .staticSwitchParameters = fallback->getArrayAs<FStaticSwitchParameter>
    //     (fnv1a("StaticSwitchParameters"), [](FPropertyValue *val) {
    //       FStaticSwitchParameter parameter{};
    //       if (auto *strct = std::get_if<FStructFallback>(&val->Data)) {
    //         parameter.value = strct->getOrDefault<bool>(fnv1a("Value"), []() { return false; });
    //       }
    //       return parameter;
    //     }),
    //     .staticComponentMaskParameters = fallback->getArrayAs<FStaticComponentMaskParameter>
    //     (fnv1a("StaticComponentMaskParameters"), [](FPropertyValue *val) {
    //       return FStaticComponentMaskParameter::get(val);
    //     })
    //   };
    // }
  };
} // namespace SAP
