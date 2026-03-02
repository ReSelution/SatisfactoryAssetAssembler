//
// Created by oleub on 20.12.25.
//

#pragma once
#include <string>

namespace SAA {
#pragma pack(push, 1)
  struct FObjectImport {
    std::string ClassPackage; // z.B. "/Script/CoreUObject"
    std::string ClassName;    // z.B. "Class"
    int32_t OuterIndex;       // Index des Outer-Objekts (oft Package)
    std::string ObjectName;   // Name des Imports
  };

#pragma pack(pop)
} // namespace SAP
