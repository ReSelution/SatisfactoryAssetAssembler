//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
  enum class EUE5Version : uint32_t {
    UE5_1 = 1004,
    UE5_2 = 1008,
    UE5_3 = 1009,
    UE5_4 = 1012,
    UE5_5 = 1013,
    UE5_7 = 1017,
  };

  enum class EUnrealEngineObjectUE5Version : uint32_t {
    // The original UE5 version, starts from 1000
    INITIAL_VERSION = 1000,

    // Support stripping names that are not referenced from export data
    NAMES_REFERENCED_FROM_EXPORT_DATA,

    // Added a payload table of contents to the package summary
    PAYLOAD_TOC,

    // Added data to identify references from and to optional package
    OPTIONAL_RESOURCES,

    // Large world coordinates converts a number of core types to double components by default
    LARGE_WORLD_COORDINATES,

    // Remove package GUID from FObjectExport
    REMOVE_OBJECT_EXPORT_PACKAGE_GUID,

    // Add IsInherited to the FObjectExport entry
    TRACK_OBJECT_EXPORT_IS_INHERITED,

    // Replace FName asset path in FSoftObjectPath with (package name, asset name) pair FTopLevelAssetPath
    FSOFTOBJECTPATH_REMOVE_ASSET_PATH_FNAMES,

    // Add a soft object path list to the package summary for fast remap
    ADD_SOFTOBJECTPATH_LIST,

    // Added bulk/data resource table
    DATA_RESOURCES,

    // Added script property serialization offset to export table entries for saved, versioned packages
    SCRIPT_SERIALIZATION_OFFSET,

    // Adding property tag extension,
    // Support for overridable serialization on UObject,
    // Support for overridable logic in containers
    PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION,

    // Added property tag complete type name and serialization type
    PROPERTY_TAG_COMPLETE_TYPE_NAME,

    // Changed UE::AssetRegistry::WritePackageData to include PackageBuildDependencies
    ASSETREGISTRY_PACKAGEBUILDDEPENDENCIES,

    // Added meta data serialization offset to for saved, versioned packages
    METADATA_SERIALIZATION_OFFSET,

    // Added VCells to the object graph
    VERSE_CELLS,

    // Changed PackageFileSummary to write FIoHash PackageSavedHash instead of FGuid Guid
    PACKAGE_SAVED_HASH,

    // OS shadow serialization of subobjects
    OS_SUB_OBJECT_SHADOW_SERIALIZATION,

    // Adds a table of hierarchical type information for imports in a package
    IMPORT_TYPE_HIERARCHIES,

    // -----<new versions can be added before this line>-------------------------------------------------
    AUTOMATIC_VERSION_PLUS_ONE,
    AUTOMATIC_VERSION = AUTOMATIC_VERSION_PLUS_ONE - 1
  };

  inline bool operator>=(EUE5Version lhs, EUnrealEngineObjectUE5Version rhs) {
    return static_cast<uint32_t>(lhs) >= static_cast<uint32_t>(rhs);
  }

  inline bool operator>(EUE5Version lhs, EUnrealEngineObjectUE5Version rhs) {
    return static_cast<uint32_t>(lhs) > static_cast<uint32_t>(rhs);
  }

  inline bool operator<=(EUE5Version lhs, EUnrealEngineObjectUE5Version rhs) {
    return static_cast<uint32_t>(lhs) <= static_cast<uint32_t>(rhs);
  }

  inline bool operator<(EUE5Version lhs, EUnrealEngineObjectUE5Version rhs) {
    return static_cast<uint32_t>(lhs) < static_cast<uint32_t>(rhs);
  }

#define SAP_ASSERT_UE5_MIN(owner, version, msg)                                                                        \
  assert((owner)->mappings()->pkgVersion.FileVersionUE5 >= EUE5Version::version && msg)

#define SAP_ASSERT_UE5_MAX(owner, version, msg)                                                                        \
  assert((owner)->mappings()->pkgVersion.FileVersionUE5 < EUE5Version::version && msg)

#define SAP_ASSERT_UE5_BEFORE(owner, version, msg)                                                                     \
  assert((owner)->mappings()->pkgVersion.FileVersionUE5 < EUE5Version::version && msg)

#define SAP_ASSERT_UE5_AT_LEAST(owner, version, msg)                                                                   \
  assert((owner)->mappings()->pkgVersion.FileVersionUE5 >= EUE5Version::version && msg)

#pragma pack(push, 1)
  struct FPackageFileVersion {
    uint32_t FileVersionUE4;    // 4 Bytes
    EUE5Version FileVersionUE5; // 4 Bytes
  };
#pragma pack(pop)
} // namespace SAP
