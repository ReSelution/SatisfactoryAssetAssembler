//
// Created by oleub on 01.02.26.
//

#include "vfs_entry.hpp"
#include "IOStoreReader.hpp"
#include "PAKReader.hpp"
#include "city.h"

namespace SAA {
  uint64_t UnifiedVfsEntry::hashPath(const std::string_view path) {
    char16_t buffer[512];
    const size_t len = std::min(path.length(), (size_t) 511);

    for (size_t i = 0; i < len; ++i) {
      // Lowercase + UTF16-Promoting in einem Schritt
      const auto uc = static_cast<unsigned char>(path[i]);
      char lc = static_cast<char>(uc + (((uc - 'A' < 26u) ? 1u : 0u) << 5));
      buffer[i] = static_cast<char16_t>(lc);
    }

    // CityHash64 direkt auf dem lokalen Buffer ausführen
    return CityHash64(reinterpret_cast<const char *>(buffer), len * sizeof(char16_t));
  }

  std::span<uint8_t> IOVfsEntry::read(HeapStorage *storage) const { return owningReader->readChunk(storage, chunkId); }

  std::span<uint8_t> PAKVfsEntry::read(HeapStorage *storage) const { return owningReader->readFile(storage,index); }
} // namespace SAP
