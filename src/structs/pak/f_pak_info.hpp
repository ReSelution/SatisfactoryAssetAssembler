//
// Created by oleub on 01.02.26.
//

#pragma once
#include <vector>

#include "Decompressor.hpp"
#include "e_pak_file_version.hpp"

#include "core/f_guid.hpp"

#include "io/f_io_hash.hpp"

namespace SAP {
  constexpr uint32_t PAK_FILE_MAGIC = 0x5A6F12E1;

  struct FPakInfo {
    FGuid encryptionGuid;
    bool encryptionIndex;
    uint32_t magic;
    EPakFileVersion version;
    int64_t indexOffset;
    int64_t indexSize;
    FSHAHash hash;
    std::vector<CompressionMethod> compressionMethods;
  };
}
