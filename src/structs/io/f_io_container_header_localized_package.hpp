//
// Created by oleub on 20.12.25.
//

#pragma once
#include "core/name.hpp"
#include "core/package.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FIoContainerHeaderLocalizedPackage {
    FPackageId SourcePackageId;
    FMappedName SourcePackageName; // FMappedName -> String
  };

#pragma pack(pop)
} 
