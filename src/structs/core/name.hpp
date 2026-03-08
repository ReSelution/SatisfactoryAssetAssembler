//
// Created by oleub on 20.12.25.
//

#pragma once
#include <charconv>
#include <cstdint>
#include <string>
#include <vector>
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

    [[nodiscard]] std::string_view fromNameMap(std::span<std::string_view> map, SC::ChaosSlab *storage) const {
      auto index = NameIndex();

      if (index < 0 || static_cast<size_t>(index) >= map.size()) {
        return "None";
      }

      std::string_view base = map[index];
      if (extraIndex == 0) {
        return base;
      }
      constexpr size_t maxSuffix = 1 + 10;
      const uint32_t baseSize = base.size();
      size_t totalSize = baseSize + maxSuffix;
      auto buf = storage->allocateSpan<char>(totalSize);
      std::memcpy(buf.data(), base.data(), baseSize);

      char* ptr = buf.data();
      ptr += base.size();

      *ptr = '_';
      ptr++;

    auto [next, ec] = std::to_chars(ptr, buf.data() + buf.size(), extraIndex - 1);
    if (ec != std::errc{}) {
        return base;
    }
      size_t finalLen = next - buf.data();
    return {buf.data(), finalLen};
    }

    [[nodiscard]] uint32_t NameIndex() const { return nameIndex & IndexMask; }

    [[nodiscard]] EMappedNameType type() const {
      return static_cast<EMappedNameType>((nameIndex & TypeMask) >> TypeShift);
    }

    [[nodiscard]] bool isGlobal() const { return ((nameIndex & TypeMask) >> TypeShift) != 0; }
  };

  struct FName {
    uint32_t index;
    uint32_t number;
    std::string plainText;
    std::string_view plainTextView;

    [[nodiscard]] std::string_view text() {
      if (number == 0) {
        return plainTextView;
      };
      if (plainText.empty()) {
        plainText = std::string(plainTextView) + "_" + std::to_string(number - 1);
      }
      return plainText;
    }

    [[nodiscard]] bool isNone() const { return plainTextView == "None"; }
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
} // namespace SAP
