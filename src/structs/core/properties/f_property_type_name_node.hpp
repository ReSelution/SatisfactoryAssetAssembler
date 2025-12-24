//
// Created by oleub on 20.12.25.
//

#pragma once
#include "structs/core/name.hpp"

namespace SAP {
#pragma pack(push, 1)
  struct FPropertyTypeNameNode {
    FName Name;
    int32_t InnerCount = 0;
  };
#pragma pack(pop)
}
