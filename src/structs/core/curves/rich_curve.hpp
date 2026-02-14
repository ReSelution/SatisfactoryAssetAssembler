//
// Created by oleub on 22.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  enum class ERichCurveInterpMode : uint8_t {
    /** Use linear interpolation between values. */
    RCIM_Linear,
    /** Use a constant value. Represents stepped values. */
    RCIM_Constant,
    /** Cubic interpolation. See TangentMode for different cubic interpolation options. */
    RCIM_Cubic,
    /** No interpolation. */
    RCIM_None
  };
  enum class ERichCurveTangentMode : uint8_t {
    /** Automatically calculates tangents to create smooth curves between values. */
    RCTM_Auto,

    /** User specifies the tangent as a unified tangent where the two tangents are locked to each other, presenting a
       consistent curve before and after. */
    RCTM_User,

    /** User specifies the tangent as two separate broken tangents on each side of the key which can allow a sharp
       change in evaluation before or after. */
    RCTM_Break,

    /** No tangents. */
    RCTM_None
  };

  enum class ERichCurveTangentWeightMode : uint8_t {
    /** Don't take tangent weights into account. */
    RCTWM_WeightedNone,

    /** Only take the arrival tangent weight into account for evaluation. */
    RCTWM_WeightedArrive,

    /** Only take the leaving tangent weight into account for evaluation. */
    RCTWM_WeightedLeave,

    /** Take both the arrival and leaving tangent weights into account for evaluation. */
    RCTWM_WeightedBoth
  };
#pragma pack(push, 1)
  struct FRichCurveKey {
    ERichCurveInterpMode interpMode;
    ERichCurveTangentMode tangentMode;
    ERichCurveTangentWeightMode tangentWeightMode;
    float time;
    float value;
    float arriveTangent;
    float arriveTangentWeight;
    float leaveTangent;
    float leaveTangentWeight;
  };
#pragma pack(pop)
} // namespace SAP
