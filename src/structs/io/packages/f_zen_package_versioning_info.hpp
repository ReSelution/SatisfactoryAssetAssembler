//
// Created by oleub on 20.12.25.
//

#pragma once
#include "f_custom_version.hpp"

namespace SAP {

  enum class EZenPackageVersion : uint32_t {
    Initial,
    DataResourceTable,
    ImportedPackageNames,
    ExportDependencies,

    LatestPlusOne,
    Latest = LatestPlusOne - 1
  };

#pragma pack(push, 1)
  struct FZenPackageVersioningInfo {
    EZenPackageVersion ZenVersion;
    FPackageFileVersion PackageVersion;
    int32_t LicenseeVersion;
    FCustomVersion CustomVersions;
  };
#pragma pack(pop)

} // namespace SAP
