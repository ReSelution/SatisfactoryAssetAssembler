//
// Created by oleub on 11.01.26.
//

#include "material_interface.hpp"

namespace SAA {
  void MaterialInterface::parse() {
    UnrealMaterial::parse();
    // m_textureStreamingData = getArrayAs<FMaterialTextureInfo>
    //     (static_cast<uint32_t>(EMaterialInterfaceProperty::TextureStreamingData), [](FPropertyValue *val) {
    //       return FMaterialTextureInfo::parse(val);
    //     });
    // m_flattenedTexture = getObjectAs<Texture>
    //     (fnv1a("FlattenedTexture"), []() { return std::weak_ptr<Texture>(); });

    if (m_cursor.readBool()) {
      cachedExpressionData = parseFallbackStruct(m_cursor);
    }
  }
} 
