//
// Created by oleub on 19.12.25.
//

#pragma once
#include <cstdint>

namespace SAP{
  enum class TextureFilter: uint32_t {
    TF_Nearest = fnv1a("TF_Nearest"),
    TF_Bilinear = fnv1a("TF_Bilinear"),
    TF_Trilinear = fnv1a("TF_Trilinear"),
    TF_Default = fnv1a("TF_Default"),
    TF_MAX = fnv1a("TF_MAX")
  };
}
