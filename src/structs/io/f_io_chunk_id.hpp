//
// Created by oleub on 20.12.25.
//

#pragma once

#include <cstdint>

#include "core/package.hpp"

namespace SAA {
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
  struct hash<SAA::FIoChunkId> {
    size_t operator()(const SAA::FIoChunkId &id) const noexcept {
      return id.id;
    }
  };
} // namespace std
