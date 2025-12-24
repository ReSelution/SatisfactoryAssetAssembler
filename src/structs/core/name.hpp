//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "functions.hpp"

namespace SAP {
#pragma pack(push, 1)
  enum class EMappedNameType : uint32_t {
    Package   = 0,
    Container = 1,
    Global    = 2
  };

  struct FMappedName {
    static constexpr int IndexBits = 30;
    static constexpr uint32_t IndexMask = (1u << IndexBits) - 1u;
    static constexpr uint32_t TypeMask = ~IndexMask;
    static constexpr int TypeShift = IndexBits;

    uint32_t nameIndex;
    uint32_t extraIndex;

    [[nodiscard]] std::string fromNameMap(const std::vector<std::string> &map) const {
      if (extraIndex == 0) return {map[NameIndex()]};

      return {map[NameIndex()] + "_" + std::to_string(extraIndex - 1)};
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

    [[nodiscard]] std::string text() const {
      return number == 0 ? plainText : plainText + "_" + std::to_string(number - 1);
    }
    [[nodiscard]] bool isNone() const { return text()  == "None"; }
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


  inline std::vector<std::string> loadNameBatch(const std::vector<uint8_t> &buffer, size_t &cursor) {
    if (cursor + sizeof(uint32_t) > buffer.size()) return {};
    auto count = readPrimitive<uint32_t>(buffer, cursor);

    std::vector<std::string> entries = {};
    if (count == 0) return entries;

    cursor += sizeof(uint32_t);         // numStringBytes
    cursor += sizeof(uint64_t);         // hashVersion
    cursor += count * sizeof(uint64_t); // hashes

    auto headers = readArray<FSerializedNameHeader>(buffer, cursor, count);

    entries.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
      const auto &header = headers[i];
      const uint32_t length = header.Length();

      std::string name;
      if (header.IsUtf16()) {
        auto chars = readArray<char16_t>(buffer, cursor, length);
        name = utf16ToUtf8(chars);
      } else {
        auto bytes = readArray<uint8_t>(buffer, cursor, length);
        name.assign(reinterpret_cast<char *>(bytes.data()), length);
      }
      entries.emplace_back(std::move(name));
    }
    return entries;
  }

#pragma pack(pop)
}
