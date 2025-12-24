//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

#include "core/flags.hpp"
#include "core/name.hpp"
#include "io/packages/f_export_map_entry.hpp"

namespace SAP {
#pragma pack(push, 1)
  struct FZenPackageSummary {
    uint32_t bHasVersioningInfo;
    uint32_t HeaderSize;
    FMappedName Name;
    EPackageFlags PackageFlags;
    uint32_t CookedHeaderSize;
    int32_t ImportedPublicExportHashesOffset;
    int32_t ImportMapOffset;
    int32_t ExportMapOffset;
    int32_t ExportBundleEntriesOffset;
    int32_t DependencyBundleHeadersOffset;
    int32_t DependencyBundleEntriesOffset;
    int32_t ImportedPackageNamesOffset;

    [[nodiscard]] uint32_t exportCount() const { return (ExportBundleEntriesOffset - ExportMapOffset) / sizeof(FExportMapEntry); }

    [[nodiscard]] uint32_t importCount() const { return (ExportMapOffset - ImportMapOffset) / sizeof(FPackageObjectIndex); }
  };
#pragma pack(pop)
}
