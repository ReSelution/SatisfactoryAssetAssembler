//
// Created by oleub on 11.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  enum class ETranslucencyLightingMode : uint32_t {
    TLM_VolumetricNonDirectional = fnv1a("TLM_VolumetricNonDirectional"),
    TLM_VolumetricDirectional = fnv1a("TLM_VolumetricDirectional"),
    TLM_VolumetricPerVertexNonDirectional = fnv1a("TLM_VolumetricPerVertexNonDirectional"),
    TLM_VolumetricPerVertexDirectional = fnv1a("TLM_VolumetricPerVertexDirectional"),
    TLM_Surface = fnv1a("TLM_Surface"),
    TLM_SurfacePerPixelLighting = fnv1a("TLM_SurfacePerPixelLighting"),
    TLM_MAX = fnv1a("TLM_MAX")
  };
}
