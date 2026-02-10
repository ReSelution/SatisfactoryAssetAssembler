//
// Created by oleub on 24.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

#include "../../../util/byte_cursor.hpp"

namespace SAP {
   enum class ERichCurveCompressionFormat : uint8_t
  {
    /** No keys are present */
    RCCF_Empty,

    /** All keys use constant interpolation */
    RCCF_Constant,

    /** All keys use linear interpolation */
    RCCF_Linear,

    /** All keys use cubic interpolation */
    RCCF_Cubic,

    /** Keys use mixed interpolation modes */
    RCCF_Mixed,

    /** Keys use weighted interpolation modes */
    RCCF_Weighted,
};

   enum class ERichCurveKeyTimeCompressionFormat : uint8_t
  {
    /** Key time is quantized to 16 bits */
    RCKTCF_uint16,

    /** Key time uses full precision */
    RCKTCF_float32,
};
   enum class ERichCurveExtrapolation : uint8_t
  {
    /** Repeat the curve without an offset. */
    RCCE_Cycle,
    /** Repeat the curve with an offset relative to the first or last key's value. */
    RCCE_CycleWithOffset,
    /** Sinusoidally extrapolate. */
    RCCE_Oscillate,
    /** Use a linearly increasing value for extrapolation.*/
    RCCE_Linear,
    /** Use a constant value for extrapolation */
    RCCE_Constant,
    /** No Extrapolation */
    RCCE_None
};

  struct FCompressedRichCurve {
    ERichCurveCompressionFormat compression_format;
    ERichCurveKeyTimeCompressionFormat key_time_compression_format;
    ERichCurveExtrapolation preInfinityExtrapolation;
    ERichCurveExtrapolation postInfinityExtrapolation;
    int32_t numKeyOrConstants;
     std::span<const uint8_t> compressionKeys;

    static FCompressedRichCurve get(ByteCursor &cursor) {
       return FCompressedRichCurve {
       .compression_format = cursor.read<ERichCurveCompressionFormat>(),
         .key_time_compression_format = cursor.read<ERichCurveKeyTimeCompressionFormat>(),
         .preInfinityExtrapolation = cursor.read<ERichCurveExtrapolation>(),
         .postInfinityExtrapolation = cursor.read<ERichCurveExtrapolation>(),
         .numKeyOrConstants = cursor.read<int32_t>(),
         .compressionKeys = cursor.readArray<uint8_t>(),
       };
    }
  };
}
