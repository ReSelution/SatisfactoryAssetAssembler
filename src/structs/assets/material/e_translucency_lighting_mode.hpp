//
// Created by oleub on 11.01.26.
//

#pragma once
#include <cstdint>
#include "sc/hash/hash.hpp"

namespace SAA {
    enum class ETranslucencyLightingMode : uint64_t {
        TLM_VolumetricNonDirectional          = "TLM_VolumetricNonDirectional"_h,
        TLM_VolumetricDirectional             = "TLM_VolumetricDirectional"_h,
        TLM_VolumetricPerVertexNonDirectional = "TLM_VolumetricPerVertexNonDirectional"_h,
        TLM_VolumetricPerVertexDirectional    = "TLM_VolumetricPerVertexDirectional"_h,
        TLM_Surface                           = "TLM_Surface"_h,
        TLM_SurfacePerPixelLighting           = "TLM_SurfacePerPixelLighting"_h,
        TLM_MAX                               = "TLM_MAX"_h
    };
}
