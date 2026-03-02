//
// Created by oleub on 09.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

namespace SAA {

  struct FWeightedRandomSampler {
    std::span<const float> probabilities;
    std::span<const int32_t> alias;
    float totlalWeight;
  };
} // namespace SAP
