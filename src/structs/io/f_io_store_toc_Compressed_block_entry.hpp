//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push,1)
  struct FIoStoreTocCompressedBlockEntry {
    uint64_t compressedOffset: 40;
    uint32_t compressedSize  : 24;
    uint32_t uncompressedSize: 24;
    uint8_t compressionMethod;
  };
#pragma pack(pop)

  static_assert
  (sizeof(FIoStoreTocCompressedBlockEntry) == 12,
   "Die Größe von FIoStoreTocCompressedBlockEntry muss exakt 12 Bytes betragen! "
   "Möglicherweise wurden Padding-Bytes hinzugefügt. Prüfe #pragma pack.");
  // namespace SAP
}
