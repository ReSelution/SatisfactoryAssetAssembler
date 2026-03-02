//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAA {
  enum class TextureCompressionSettings : uint32_t {
    TC_Default = fnv1a("TC_Default"),
    TC_Normalmap = fnv1a("TC_Normalmap"),
    TC_Masks = fnv1a("TC_Masks"),
    TC_Grayscale = fnv1a("TC_Grayscale"),
    TC_Displacementmap = fnv1a("TC_Displacementmap"),
    TC_VectorDisplacementmap = fnv1a("TC_VectorDisplacementmap"),
    TC_HDR = fnv1a("TC_HDR"),
    TC_EditorIcon = fnv1a("TC_EditorIcon"),
    TC_Alpha = fnv1a("TC_Alpha"),
    TC_DistanceFieldFont = fnv1a("TC_DistanceFieldFont"),
    TC_HDR_Compressed = fnv1a("TC_HDR_Compressed"),
    TC_BC7 = fnv1a("TC_BC7"),
    TC_HalfFloat = fnv1a("TC_HalfFloat"),
    TC_LQ = fnv1a("TC_LQ"),
    TC_EncodedReflectionCapture = fnv1a("TC_EncodedReflectionCapture"),
    TC_SingleFloat = fnv1a("TC_SingleFloat"),
    TC_HDR_F32 = fnv1a("TC_HDR_F32"),
    TC_MAX = fnv1a("TC_MAX")
  };
}
