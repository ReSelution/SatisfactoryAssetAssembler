//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push, 1)

  struct FGuid {
    uint32_t A, B, C, D;
    static FGuid fromUint32(uint32_t value) {
      FGuid guid{.A = value, .B = value, .C = value, .D = value};
      return guid;
    }
    bool operator==(const FGuid &other) const { return A == other.A && B == other.B && C == other.C && D == other.D; }
    bool operator!=(const FGuid &other) const { return !(*this == other); }
  };
#pragma pack(pop)
} // namespace SAP
