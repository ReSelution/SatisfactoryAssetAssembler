//
// Created by oleub on 19.12.25.
//

#pragma once
#include <cstdint>

namespace SAP{
  enum class TextureFilter: uint8_t {
    TF_Nearest,
    TF_Bilinear,
    TF_Trilinear,
    /** Use setting from the Texture Group. */
    TF_Default,
    TF_MAX,
  };
}
