//
// Created by oleub on 13.01.26.
//

#pragma once
#include "e_material_parameter_association.hpp"

#include "core/name.hpp"

namespace SAP {
  struct FMaterialParameterInfo {
    FName name;
    EMaterialParameterAssociation association;
    int32_t index;
    // static FMaterialParameterInfo get(FStructFallback *fallback) {
    //   FMaterialParameterInfo p;
    //     p.name = fallback->getOrDefault<FName>(fnv1a("Name"), [](){return FName();});
    //     p.association = fallback->getOrDefaultEnum<EMaterialParameterAssociation>(fnv1a("Association"), [](){return EMaterialParameterAssociation::BlendParameter;});
    //     p.index = fallback->getOrDefault<int32_t>(fnv1a("Index"), [](){return 0;});
    //
    // return p;
    // }
  };
}
