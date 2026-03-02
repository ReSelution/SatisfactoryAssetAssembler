//
// Created by oleub on 20.12.25.
//

#pragma once
#include <array>
#include <cstdint>

namespace SAA {
  constexpr uint32_t FSHA_HASH_SIZE = 20;

#pragma pack(push, 1)

  struct FSHAHash {

    std::array<uint8_t, FSHA_HASH_SIZE> hash;
  };
#pragma pack(pop)
} // namespace SAP
