//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push,1)

  struct FIoFileIndexEntry {
    uint32_t name; // Index in String Table
    uint32_t nextFileEntry;
    uint32_t userData;
  }; // Exakt 12 Bytes

#pragma pack(pop)

} // SAP