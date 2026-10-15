//
// Created by oleub on 11.01.26.
//

#pragma once
#include <cstdint>
#include <cstring>

// F16C converts in one instruction. GCC and Clang name the target feature, MSVC only has it as part of AVX2.
#if defined(__F16C__) || (defined(_MSC_VER) && defined(__AVX2__))
#define SAA_HAS_F16C 1
#include <immintrin.h>
#endif

namespace SAA {
#pragma pack(push, 1)
  // An IEEE 754 binary16 as it lies in the file. Not std::float16_t: Clang does not implement P1467 (no
  // __STDCPP_FLOAT16_T__ up to Clang 23), so libstdc++ does not declare it, and the Microsoft STL has no <stdfloat>.
  struct half {
    uint16_t data;

    [[nodiscard]] float toFloat() const {
#ifdef SAA_HAS_F16C
      return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(data)));
#else
      return toFloatSoftware();
#endif
    }

    // Without F16C, -march=x86-64-v2 for example. Gives the same value as the instruction for every input, except
    // that a signaling NaN stays signaling where the instruction makes it quiet.
    [[nodiscard]] float toFloatSoftware() const {
      uint32_t sign = (data & 0x8000) << 16;
      uint32_t exp  = (data & 0x7C00) >> 10;
      uint32_t mant = (data & 0x03FF) << 13;

      if (exp == 0x1F) { // Inf or NaN
        exp = 0xFF;
      } else if (exp == 0) { // Zero or subnormal
        if (mant != 0) {     // Subnormal
          exp = 0x71;
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
} // namespace SAA
