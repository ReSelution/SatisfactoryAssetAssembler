//
// Created by oleub on 20.12.25.
//

#pragma once

#include <cstdint>
#include <string>
#include "hash/hash.hpp"
#include "name.hpp"
#include "city_inline.hpp"

namespace SAA {
  enum class EType : uint8_t {
    Export = 0, ScriptImport = 1, PackageImport = 2, Null = 3, TypeCount = Null
  };

#pragma pack(push, 1)

  struct FPackageId {
    uint64_t Id;

    static FPackageId fromName(const std::string_view name) {
      char16_t buffer[512];
      const size_t len = std::min(name.length(), (size_t) 511);

      for (size_t i = 0; i < len; ++i) {
        const auto uc = static_cast<uint8_t>(name[i]);
        uint8_t lc = (uc >= 'A' && uc <= 'Z') ? (uc + 32) : uc;
        buffer[i] = static_cast<char16_t>(lc);
      }


      return FPackageId(CityHash64_inline(reinterpret_cast<char *>(buffer), len * sizeof(char16_t)));
    }

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
}

namespace std {
  template<>
  struct hash<SAA::FPackageId> {
    size_t operator()(const SAA::FPackageId &id) const noexcept { return static_cast<size_t>(id.Id); }
  };

  template<>
  struct hash<SAA::FPackageObjectIndex> {
    size_t operator()(const SAA::FPackageObjectIndex &index) const noexcept {
      return SC::hash(index.TypeAndId);
    }
  };
} // namespace std
