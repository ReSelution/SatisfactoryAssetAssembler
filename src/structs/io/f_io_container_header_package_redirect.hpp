//
// Created by oleub on 20.12.25.
//

#pragma once
#include "core/name.hpp"
#include "core/package.hpp"

namespace SAP {
#pragma pack(push, 1)
  struct FIoContainerHeaderPackageRedirect {
    FPackageId SourcePackageId;
    FPackageId TargetPackageId;
    FMappedName SourcePackageName;
  };

#pragma pack(pop)
} // namespace SAP
