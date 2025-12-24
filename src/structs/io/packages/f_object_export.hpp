//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <string>

namespace SAP {
#pragma pack(push, 1)
  struct FObjectExport {
    int32_t ClassIndex; // Referenz auf den Typ (Import oder Export)
    int32_t SuperIndex; // Parent Class
    int32_t TemplateIndex;
    int32_t OuterIndex;
    std::string ObjectName;

    uint32_t ObjectFlags;

    // Serialisierte Daten
    int64_t SerialSize;   // Größe der Daten im Body
    int64_t SerialOffset; // Offset in der Datei (.uasset oder .uexp)

    bool bNotForClient;
    bool bNotForServer;
  };

#pragma pack(pop)
}
