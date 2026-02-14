//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

#include "functions.hpp"

namespace SAP {
  constexpr uint64_t decode40Bit(const std::array<uint8_t, 5> &data) {
    return static_cast<uint64_t>(data[4]) | (static_cast<uint64_t>(data[3]) << 8) |
           (static_cast<uint64_t>(data[2]) << 16) | (static_cast<uint64_t>(data[1]) << 24) |
           (static_cast<uint64_t>(data[0]) << 32);
  }
#pragma pack(push, 1)
  struct FIoOffsetAndLength {
    std::array<uint8_t, 5> offsetStorage;
    std::array<uint8_t, 5> lengthStorage;

    uint64_t offset() { return decode40Bit(offsetStorage); }

    uint64_t length() { return decode40Bit(lengthStorage); }
  };
#pragma pack(pop)
} // namespace SAP
