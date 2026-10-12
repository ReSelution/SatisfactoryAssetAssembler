//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <span>
#include <string_view>

namespace sc {
  class MonotonicResource;
}

namespace SAA {
#pragma pack(push, 1)
  enum class EMappedNameType : uint32_t { Package = 0, Container = 1, Global = 2 };

  struct FMappedName {
    static constexpr int IndexBits      = 30;
    static constexpr uint32_t IndexMask = (1u << IndexBits) - 1u;
    static constexpr uint32_t TypeMask  = ~IndexMask;
    static constexpr int TypeShift      = IndexBits;

    uint32_t nameIndex;
    uint32_t extraIndex;

    [[nodiscard]] std::string_view fromNameMap(std::span<std::string_view> map, sc::MonotonicResource *storage) const;

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
    // The whole name. For a numbered name that includes the suffix ("Foo_1" for number 2), ByteCursor::readFName()
    // builds it once when the name is read.
    std::string_view textView;
    uint32_t index;
    uint32_t number;

    [[nodiscard]] std::string_view text() const { return textView; }

    // base with the suffix of number appended, stored in storage. number must not be 0.
    [[nodiscard]] static std::string_view numbered(std::string_view base, uint32_t number,
                                                   sc::MonotonicResource *storage);

    // Decided by the text alone: index 0 of a package name map is an ordinary name (often the first one in
    // alphabetical order, "Amount" for example). A default constructed FName has no text and counts as None.
    [[nodiscard]] bool isNone() const { return textView.empty() || textView == "None"; }
  };
} // namespace SAA
