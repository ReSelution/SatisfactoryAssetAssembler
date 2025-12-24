//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
  enum class TextureCompressionSettings: uint8_t {
    TC_Default,
    TC_Normalmap,
    TC_Masks,
    TC_Grayscale,
    TC_Displacementmap,
    TC_VectorDisplacementmap,
    TC_HDR,
    TC_EditorIcon,
    TC_Alpha,
    TC_DistanceFieldFont,
    TC_HDR_Compressed,
    TC_BC7,
    TC_HalfFloat,
    TC_LQ,
    TC_EncodedReflectionCapture,
    TC_SingleFloat,
    TC_HDR_F32,
    TC_MAX,
  };
}
