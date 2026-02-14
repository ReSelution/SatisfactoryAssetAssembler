//
// Created by oleub on 11.01.26.
//

#pragma once
#include "material_interface.hpp"

#include "assets/material/f_material_instance_base_property_overrides.hpp"
#include "assets/material/f_static_parameter_set.hpp"

#include "ue/object_factory.hpp"

namespace SAP {
  class MaterialInstance : public MaterialInterface {
    friend class MaterialExporter;

  public:
    MaterialInstance(std::string_view classname = "MaterialInstance") : MaterialInterface(classname) {};
    void exportToGltf(GltfMaterial &gltfMaterial) override;

  protected:
    void parse() override;

    std::weak_ptr<UnrealMaterial> m_parent;
    bool m_HasNonUPropertyStaticParameters = false;
    FMaterialInstanceBasePropertyOverrides basePropertyOverrides{};
    FStaticParameterSet staticParameters;
    PropertyHolder cachedData;

  private:
  };

  SAP_REGISTER_ASSET(MaterialInstance)
} // namespace SAP
