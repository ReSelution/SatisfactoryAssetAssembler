//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push, 1)

  struct FIoOffsetAndLength {
    uint64_t offset: 40;
    uint64_t lenght: 40;
  };

#pragma pack(pop)

  static_assert
(sizeof(FIoOffsetAndLength) == 10,
 "Die Größe von FIoOffsetAndLength muss exakt 10 Bytes betragen! "
 "Möglicherweise wurden Padding-Bytes hinzugefügt. Prüfe #pragma pack.");

}
