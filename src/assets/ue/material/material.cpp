//
// Created by oleub on 11.01.26.
//

#include "material.hpp"

#include "assets/material/f_material_parameter_info.hpp"

#include "material/material_exporter.hpp"

namespace SAA {
  void Material::exportToGltf(GltfMaterial &outMaterial) { MaterialExporter::encodeMaterial(outMaterial, this); }

  void Material::parse() {
    MaterialInterface::parse();
    // twoSided = getOrDefault<bool>(static_cast<uint32_t>(EMaterialInterface::TwoSided), []() { return false; });
    // disableDepthTest = getOrDefault<bool>
    //     (static_cast<uint32_t>(EMaterialInterface::bDisableDepthTest), []() { return false; });
    // isMasked = getOrDefault<bool>(static_cast<uint32_t>(EMaterialInterface::bIsMasked), []() { return false; });
    // m_expressions = getArrayAs<std::weak_ptr<Object> >
    //     (static_cast<uint32_t>(EMaterialInterface::Expressions), [](FPropertyValue *val) {
    //       if (auto obj = std::get_if<std::weak_ptr<Object> >(&val->Data)) { return *obj; }
    //       return std::weak_ptr<Object>();
    //     });
    //
    // blendMode = getOrDefaultEnum<EBlendMode>
    //     (static_cast<uint32_t>(EMaterialInterface::BlendMode), []() { return EBlendMode::BLEND_Opaque; });
    //
    // translucencyLightingMode = getOrDefaultEnum<ETranslucencyLightingMode>
    //     (static_cast<uint32_t>(EMaterialInterface::TranslucencyLightingMode), []() {
    //       return ETranslucencyLightingMode::TLM_VolumetricNonDirectional;
    //     });
    // shadingModel = getOrDefaultEnum<EMaterialShadingModel>
    //     (static_cast<uint32_t>(EMaterialInterface::ShadingModel), []() { return EMaterialShadingModel::MSM_Unlit; });
    //
    // opacityMaskClipValue = getOrDefault<float>
    //     (static_cast<uint32_t>(EMaterialInterface::OpacityMaskClipValue), []() { return 0.333f; });
    //
    // if (!cachedExpressionData.has_value()) {
    //   auto iter = m_properties.find(fnv1a("CachedExpressionData"));
    //   if (iter != m_properties.end()) {
    //     cachedExpressionData = std::get<FStructFallback>(std::move(iter->second.Tag->Data));
    //   }
    // }
    // if (cachedExpressionData.has_value()) {
    // auto iter = cachedExpressionData->properties.find(fnv1a("RuntimeEntries"));
    // if (iter != cachedExpressionData->properties.end()) {
    //   auto runtimeEntries = std::get_if<FStructFallback>(&iter->second.Tag->Data);
    //   if (runtimeEntries) {
    //     for (auto &entry: runtimeEntries->properties) {
    //       Logger::logInfo
    //           ("RuntimeEntries", "Tag: {} -> {} : {}", entry.second.Name.text(), to_string
    //            (entry.second.PropertyType), to_string(entry.second.TagData->InnerType));
    //       auto *paramterInfo = std::get_if<std::vector<std::unique_ptr<FPropertyValue> > >(&entry.second.Tag->Data);
    //       for (auto &parm: *paramterInfo) {
    //         auto prop = std::get_if<FStructFallback>(&parm->Data);
    //         auto pramInfo = FMaterialParameterInfo::get(prop);
    //         Logger::logInfo
    //             ("PramInfo", "Name: {} Index: {}", pramInfo.name.text(), pramInfo.index);
    //       }
    //     }
    //   }
    // }

    // for (auto &entry: cachedExpressionData.value().properties) {
    //   Logger::logInfo
    //         ("RuntimeEntries", "Tag: {} -> {} : {}", entry.second.Name.text(), to_string
    //       (entry.second.PropertyType), to_string(entry.second.TagData->InnerType));
    //
    // }

    // m_textures = cachedExpressionData.value().getArrayAs<std::weak_ptr<Texture> >
    //     (fnv1a("ReferencedTextures"), [](FPropertyValue *val)->std::weak_ptr<Texture> {
    //       if (auto *obj = std::get_if<std::weak_ptr<Object> >(&val->Data)) {
    //         return std::dynamic_pointer_cast<Texture>(obj->lock());
    //       }
    //       return {};
    //     });
  }
} 

// SAP
