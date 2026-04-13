//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <string>
#include "functions.hpp"

namespace SAA {
#pragma pack(push, 1)
  enum class EMappedNameType : uint32_t { Package = 0, Container = 1, Global = 2 };

  struct FMappedName {
    static constexpr int IndexBits = 30;
    static constexpr uint32_t IndexMask = (1u << IndexBits) - 1u;
    static constexpr uint32_t TypeMask = ~IndexMask;
    static constexpr int TypeShift = IndexBits;

    uint32_t nameIndex;
    uint32_t extraIndex;

    [[nodiscard]] std::string_view fromNameMap(std::span<std::string_view> map, SC::ChaosBumpArena *storage) const ;

    [[nodiscard]] uint32_t NameIndex() const { return nameIndex & IndexMask; }

    [[nodiscard]] EMappedNameType type() const {
      return static_cast<EMappedNameType>((nameIndex & TypeMask) >> TypeShift);
    }

    [[nodiscard]] bool isGlobal() const { return ((nameIndex & TypeMask) >> TypeShift) != 0; }
  };

  struct FSerializedNameHeader {
    static constexpr int Size = 2;

    uint8_t data0;
    uint8_t data1;

    [[nodiscard]] bool IsUtf16() const { return (data0 & 0x80u) != 0; }

    [[nodiscard]] uint32_t Length() const { return ((data0 & 0x7Fu) << 8) + data1; }

    // Equality
    bool operator==(const FSerializedNameHeader &other) const { return data0 == other.data0 && data1 == other.data1; }

    bool operator!=(const FSerializedNameHeader &other) const { return !(*this == other); }
  };

#pragma pack(pop)

  struct FName {
    std::string_view plainTextView;
    mutable std::string_view generatedView;
    uint32_t index;
    uint32_t number;
    SC::ChaosBumpArena *slab;

    [[nodiscard]] std::string_view text() const;

    [[nodiscard]] bool isNone() const { return plainTextView == "None"; }
  };
}
