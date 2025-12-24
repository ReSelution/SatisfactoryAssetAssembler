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
      FGuid guid{ .A = value, .B = value, .C = value, .D = value };
      return guid;
    }

  };
#pragma pack(pop)
}
