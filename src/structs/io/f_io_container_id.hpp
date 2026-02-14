//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push, 1)
  struct FIoContainerId {
    uint64_t A;
    uint64_t B;
  };

#pragma pack(pop)
} // namespace SAP
