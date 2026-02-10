//
// Created by oleub on 31.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push, 1)
  struct FOptTexturePlatformData {
    uint32_t extData;
    uint32_t numMipsInTail;
  };
#pragma pack(pop)
}
