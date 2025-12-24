//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include "core/f_guid.hpp"

namespace SAP {
#pragma pack(push, 1)
  struct FCustomVersion {
    FGuid key;
    uint32_t Version;
  };
#pragma pack(pop)
}
