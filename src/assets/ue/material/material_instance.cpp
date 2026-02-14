//
// Created by oleub on 11.01.26.
//

#include "material_instance.hpp"

namespace SAP {
  void MaterialInstance::parse() {
    MaterialInterface::parse();
    // m_parent = getObjectAs<UnrealMaterial>(fnv1a("Parent"), []() { return std::shared_ptr<UnrealMaterial>(nullptr);
    // }); m_HasNonUPropertyStaticParameters = getOrDefault<bool>
    //     (fnv1a("HasNonUPropertyStaticParameters"), []() { return false; });
    // basePropertyOverrides = getStructAs<FMaterialInstanceBasePropertyOverrides>
    //     (fnv1a("BasePropertyOverrides"),
    //      [](FStructFallback *fallback) { return FMaterialInstanceBasePropertyOverrides::parse(fallback); },
    //      []() { return FMaterialInstanceBasePropertyOverrides(); });
    // staticParameters = getStructAs<FStaticParameterSet>(fnv1a("StaticParameters"),[](FStructFallback *val) {
    //   return FStaticParameterSet::getStaticParameterSet(val);
    // }, [](){return FStaticParameterSet{};} );
  }

  void MaterialInstance::exportToGltf(GltfMaterial &gltfMaterial) { MaterialInterface::exportToGltf(gltfMaterial); }
} // namespace SAP
