//
// Created by oleub on 21.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  enum class ETextFlag : uint32_t {
    Transient = (1 << 0),
    CultureInvariant = (1 << 1),
    ConvertedProperty = (1 << 2),
    Immutable = (1 << 3),
    InitializedFromString = (1 << 4), // this ftext was initialized using FromString
  };

  enum class ETextHistoryType : int8_t {
    None = -1,
    Base = 0,
    NamedFormat,
    OrderedFormat,
    ArgumentFormat,
    AsNumber,
    AsPercent,
    AsCurrency,
    AsDate,
    AsTime,
    AsDateTime,
    Transform,
    StringTableEntry,
    TextGenerator,
  };

  enum class EFormatArgumentType : int8_t {
    Int,
    UInt,
    Float,
    Double,
    Text,

    // Add new enum types at the end only! They are serialized by index.
  };
  enum class ERoundingMode : int8_t {
    /** Rounds to the nearest place, equidistant ties go to the value which is closest to an even value: 1.5 becomes 2,
       0.5 becomes 0 */
    HalfToEven,

    /** Rounds to nearest place, equidistant ties go to the value which is further from zero: -0.5 becomes -1.0, 0.5
       becomes 1.0 */
    HalfFromZero,

    /** Rounds to nearest place, equidistant ties go to the value which is closer to zero: -0.5 becomes 0, 0.5 becomes
       0. */
    HalfToZero,

    /** Rounds to the value which is further from zero, "larger" in absolute value: 0.1 becomes 1, -0.1 becomes -1 */
    FromZero,

    /** Rounds to the value which is closer to zero, "smaller" in absolute value: 0.1 becomes 0, -0.1 becomes 0 */
    ToZero,

    /** Rounds to the value which is more negative: 0.1 becomes 0, -0.1 becomes -1 */
    ToNegativeInfinity,

    /** Rounds to the value which is more positive: 0.1 becomes 1, -0.1 becomes 0 */
    ToPositiveInfinity,

    // Add new enum types at the end only! They are serialized by index.
  };

  enum class ETransformType : int8_t {
    ToLower = 0,
    ToUpper,

  };

  struct FNumberFormattingOptions {
    uint32_t AlwaysSign;
    uint32_t UseGrouping;
    ERoundingMode RoundingMode;
    int32_t MinimumIntegralDigits;
    int32_t MaximumIntegralDigits;
    int32_t MinimumFractionalDigits;
    int32_t MaximumFractionalDigits;
  };

  struct FTextHistory {
    std::string text;
  };

  struct FText {
    ETextFlag flags;
    ETextHistoryType type;
    FTextHistory textHistory;
  };
} // namespace SAP
