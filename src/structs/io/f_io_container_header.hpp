//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "core/package.hpp"
#include "f_file_package_store_entry.hpp"
#include "f_io_container_header_localized_package.hpp"
#include "f_io_container_header_package_redirect.hpp"
#include "f_io_container_header_soft_package_refenrenes.hpp"

namespace SAA {
  static constexpr uint32_t IO_CONTAINER_MAGIC = 0x496f436e; // "IoCn"

  enum class EIoContainerHeaderVersion : int32_t {
    BeforeVersionWasAdded = -1,
    Initial = 0,
    LocalizedPackages = 1,
    OptionalSegmentPackages = 2,
    NoExportInfo = 3,
    SoftPackageReferences = 4,
    SoftPackageReferencesOffset = 5,
    LatestPlusOne,
    Latest = LatestPlusOne - 1
  };

  struct FIoContainerHeader {
    uint32_t magic;
    EIoContainerHeaderVersion version;
    FPackageId containerId;

    std::span<const FPackageId> packageIds;
    std::unordered_map<FPackageId, FFilePackageStoreEntry> storeEntriesById{};

    std::span<const FPackageId> optionalSegmentPackageIds;
    std::span<FFilePackageStoreEntry> optionalSegmentStoreEntries;
    std::span<std::string_view> containerNameMap;
    std::span<const FIoContainerHeaderLocalizedPackage> localizedPackages;
    std::span<const FIoContainerHeaderPackageRedirect> packageRedirects;

    // Not Used by Satisfactory
    FIoContainerHeaderSoftPackageReferences softPackageReferences;
  };
} // namespace SAP
