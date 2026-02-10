//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

#include "functions.hpp"

namespace SAP {
#pragma pack(push, 1)
  struct FIoOffsetAndLength_raw {
    std::array<uint8_t, 5> offset;
    std::array<uint8_t, 5> length;
  };
  #pragma pack(pop)

  constexpr  uint64_t decode40Bit(const std::array<uint8_t, 5>& data) {
    return static_cast<uint64_t>(data[4]) |
           (static_cast<uint64_t>(data[3]) << 8) |
           (static_cast<uint64_t>(data[2]) << 16) |
           (static_cast<uint64_t>(data[1]) << 24) |
           (static_cast<uint64_t>(data[0]) << 32);
  }


  struct FIoOffsetAndLength {
    uint64_t offset;
    uint64_t length;

    static FIoOffsetAndLength get(const FIoOffsetAndLength_raw &raw) {
      return FIoOffsetAndLength{
        .offset = decode40Bit(raw.offset),
        .length = decode40Bit(raw.length)
      };
    }
  };
}
