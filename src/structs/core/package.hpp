//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <string>

#include "name.hpp"

namespace SAP {
  enum class EType : uint8_t { Export = 0, ScriptImport = 1, PackageImport = 2, Null = 3, TypeCount = Null };

#pragma pack(push, 1)

  struct FPackageId {
    uint64_t Id;
    static FPackageId fromName(const std::string_view name);
    bool operator==(const FPackageId &other) const { return Id == other.Id; }
  };

  struct FPackageObjectIndex {
    uint64_t TypeAndId;

    static constexpr int Size = sizeof(uint64_t);
    static constexpr int IndexBits = 62;
    static constexpr uint64_t IndexMask = (1ULL << IndexBits) - 1ULL;
    static constexpr uint64_t TypeMask = ~IndexMask;
    static constexpr int TypeShift = IndexBits;
    static constexpr uint64_t Invalid = ~0ULL;
    static const FPackageObjectIndex InvalidObjectIndex;

    [[nodiscard]] EType GetType() const { return static_cast<EType>(TypeAndId >> TypeShift); }
    [[nodiscard]] uint64_t GetValue() const { return TypeAndId & IndexMask; }
    [[nodiscard]] bool IsNull() const { return TypeAndId == Invalid; }
    [[nodiscard]] bool IsExport() const { return GetType() == EType::Export; }
    [[nodiscard]] bool IsScriptImport() const { return GetType() == EType::ScriptImport; }
    [[nodiscard]] bool IsPackageImport() const { return GetType() == EType::PackageImport; }
    [[nodiscard]] bool IsImport() const { return IsScriptImport() || IsPackageImport(); }

    bool operator==(const FPackageObjectIndex &other) const { return TypeAndId == other.TypeAndId; }
  };

  struct FPackageImportReference {
    uint32_t importedPackageIndex;
    uint32_t importedPublicExportHashIndex;

    static FPackageImportReference fromObjIndex(const FPackageObjectIndex &index) {
      return {.importedPackageIndex = static_cast<uint32_t>((index.TypeAndId & index.IndexMask) >> 32),
              .importedPublicExportHashIndex = static_cast<uint32_t>(index.TypeAndId)};
    };
  };
#pragma pack(pop)
} // namespace SAP

namespace std {
  template<>
  struct hash<SAP::FPackageId> {
    size_t operator()(const SAP::FPackageId &id) const noexcept { return std::hash<uint64_t>{}(id.Id); }
  };

  template<>
  struct hash<SAP::FPackageObjectIndex> {
    size_t operator()(const SAP::FPackageObjectIndex &index) const noexcept {
      return hash<uint64_t>{}(index.TypeAndId);
    }
  };
} // namespace std
