//
// Created by oleub on 11.01.26.
//

#include "material_instance_constant.hpp"

#include "material/material_exporter.hpp"

namespace SAP {
  void MaterialInstanceConstant::parse() {
    MaterialInstance::parse();
 //
 //    scalar_parameters = getArrayAs<FScalarParameterValue>(fnv1a("ScalarParameterValues"), [](FPropertyValue *val) {
 //        return FScalarParameterValue::get(val);
 //    });
 //    texture_parameters = getArrayAs<FTextureParameterValue>(fnv1a("TextureParameterValues"), [](FPropertyValue *val) {
 //       return FTextureParameterValue::get(val);
 //   });
 //    vector_parameters = getArrayAs<FVectorParameterValue>(fnv1a("VectorParameterValues"), [](FPropertyValue *val) {
 //     return FVectorParameterValue::get(val);
 // });
  }

  void MaterialInstanceConstant::exportToGltf(GltfMaterial &out) {
    MaterialExporter::encodeMaterial(out, this);
  }
} // SAP