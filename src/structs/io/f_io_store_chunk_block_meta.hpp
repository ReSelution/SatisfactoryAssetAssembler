//
// Created by oleub on 20.12.25.
//

#pragma once
#include "f_io_hash.hpp"

namespace SAP {
#pragma pack(push, 1)

  struct FIoStoreChunkBlockMeta {
    FIoHash hash;
  };

#pragma pack(pop)
}
