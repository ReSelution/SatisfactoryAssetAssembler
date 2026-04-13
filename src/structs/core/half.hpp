//
// Created by oleub on 11.01.26.
//

#pragma once
#include <cstdint>
#include <cstring>

namespace SAA {
#pragma pack(push, 1)
  struct half {
    uint16_t data;

    [[nodiscard]] float toFloat() const {
      uint32_t sign = (data & 0x8000) << 16;
      uint32_t exp = (data & 0x7C00) >> 10;
      uint32_t mant = (data & 0x03FF) << 13;

      if (exp == 0x1F) { // Inf oder NaN
        exp = 0xFF;
      } else if (exp == 0) { // Zero oder Subnormal
        if (mant != 0) {     // Subnormal
          exp = 0x70;
          while ((mant & 0x00800000) == 0) {
            mant <<= 1;
            exp--;
          }
          mant &= 0x007FFFFF;
        }
      } else {
        exp += 127 - 15;
      }

      uint32_t res = sign | (exp << 23) | mant;
      float out;
      std::memcpy(&out, &res, sizeof(float));
      return out;
    }
  };
#pragma pack(pop)
} 
