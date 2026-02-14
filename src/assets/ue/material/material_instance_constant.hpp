//
// Created by oleub on 11.01.26.
//

#pragma once
#include "material_instance.hpp"

#include "assets/material/f_scalar_parameter_value.hpp"
#include "assets/material/f_texture_parameter_value.hpp"
#include "assets/material/f_vector_parameter_value.hpp"

#include "ue/object_factory.hpp"

namespace SAP {

  class MaterialInstanceConstant : public MaterialInstance {
    friend class MaterialExporter;

  public:
    MaterialInstanceConstant(std::string_view classname = "MaterialInstanceConstant") : MaterialInstance(classname) {};

    void parse() override;
    void exportToGltf(GltfMaterial &out) override;

  protected:
    std::vector<FScalarParameterValue> scalar_parameters;
    std::vector<FTextureParameterValue> texture_parameters;
    std::vector<FVectorParameterValue> vector_parameters;
  };

  SAP_REGISTER_ASSET(MaterialInstanceConstant)
} // namespace SAP
