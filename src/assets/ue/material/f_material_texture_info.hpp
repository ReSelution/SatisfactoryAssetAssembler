//
// Created by oleub on 11.01.26.
//

#pragma once
#include <cstdint>

#include "logger.hpp"

#include "core/name.hpp"

#include "ue/properties/f_property_tag.hpp"

namespace SAP {
  struct FMaterialTextureInfo {
    enum class EMaterialTextureInfo : uint32_t {
      SamplingScale = fnv1a("SamplingScale"),
      UVChannelIndex = fnv1a("UVChannelIndex"),
      TextureName = fnv1a("TextureName"),
    };

    float samplingScale;
    int32_t UVChannelIndex;
    FName textureName;

    static FMaterialTextureInfo parse(FPropertyValue *val) {
      FMaterialTextureInfo info{};
      // if (auto fallbackStruct = std::get_if<FStructFallback>(&val->Data)) {
      //   info.samplingScale = fallbackStruct->getOrDefault<float>
      //       (static_cast<uint32_t>(EMaterialTextureInfo::SamplingScale), [] { return 0.0f; });
      //
      //   info.UVChannelIndex = fallbackStruct->getOrDefault<int32_t>
      //       (static_cast<uint32_t>(EMaterialTextureInfo::UVChannelIndex), [] { return 0; });
      //
      //   info.textureName = fallbackStruct->getOrDefault<FName>
      //       (static_cast<uint32_t>(EMaterialTextureInfo::TextureName), [] { return FName{}; });
      // }
      return info;
    }
  };
} // namespace SAP
