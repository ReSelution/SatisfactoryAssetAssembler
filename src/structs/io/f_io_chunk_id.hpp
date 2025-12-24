//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

#include "core/package.hpp"

namespace SAP {
#pragma pack(push, 1)

  struct FIoChunkId {
    uint64_t id;
    uint16_t chunkIndex;
    uint8_t padding;
    uint8_t chunkType;

    [[nodiscard]] FPackageId asPackageId() const { return FPackageId{.Id = id}; }

    bool operator==(const FIoChunkId &other) const {
      return id == other.id && chunkIndex == other.chunkIndex && chunkType == other.chunkType;
    }
  };
#pragma pack(pop)
}

namespace std {
  template<>
  struct hash<SAP::FIoChunkId> {
    size_t operator()(const SAP::FIoChunkId &id) const noexcept {
      size_t hsh = std::hash<uint64_t>{}(id.id);
      hsh ^= std::hash<uint32_t>{}(id.chunkIndex) + 0x9e3779b9 + (hsh << 6) + (hsh >> 2);
      hsh ^= std::hash<uint8_t>{}(id.chunkType) + 0x9e3779b9 + (hsh << 6) + (hsh >> 2);
      return hsh;
    }
  };
}
