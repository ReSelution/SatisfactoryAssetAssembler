//
// Created by oleub on 01.02.26.
//

#include "vfs_entry.hpp"
#include "IOStoreReader.hpp"
#include "PAKReader.hpp"
#include "city.h"
#include "vfs.hpp"

namespace SAA {
  uint64_t UnifiedVfsEntry::hashPath(const std::string_view path) {

    char16_t buffer[MAX_PATH_SIZE];
    const size_t len = std::min(path.length(), MAX_PATH_SIZE);

    for (size_t i = 0; i < len; ++i) {
      const auto uc = static_cast<char>(path[i]);
      char lc = (uc >= 'A' && uc <= 'Z') ? static_cast<char>(uc + 32) : uc;
      buffer[i] = static_cast<char16_t>(lc);
    }

    return CityHash64(reinterpret_cast<const char *>(buffer), len * sizeof(char16_t));
  }

  std::span<uint8_t> IOVfsEntry::read(SC::ChaosSlab *storage) const { return owningReader->readChunk(storage, chunkId); }

  std::span<uint8_t> PAKVfsEntry::read(SC::ChaosSlab *storage) const { return owningReader->readFile(storage,index); }
} // namespace SAP
