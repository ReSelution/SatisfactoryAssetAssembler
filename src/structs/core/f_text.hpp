//
// Created by oleub on 21.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  enum class ETextFlag : uint32_t {
    Transient             = (1 << 0),
    CultureInvariant      = (1 << 1),
    ConvertedProperty     = (1 << 2),
    Immutable             = (1 << 3),
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

  struct FTextHistory {
    std::string text;
  };

  struct FText {
    ETextFlag flags;
    ETextHistoryType type;
    FTextHistory textHistory;

  };
}
