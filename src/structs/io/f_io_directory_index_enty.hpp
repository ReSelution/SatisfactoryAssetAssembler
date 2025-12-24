//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push, 1)

  struct FIoDirectoryIndexEntry {
    uint32_t name; // Index in String Table
    uint32_t firstChildEntry;
    uint32_t nextSiblingEntry;
    uint32_t firstFileEntry;
  }; // Exakt 16 Bytes


#pragma pack(pop)
}
