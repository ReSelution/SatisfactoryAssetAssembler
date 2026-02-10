//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  struct FPackedNormal {
    uint32_t data;
    [[nodiscard]] float x() const { return static_cast<float>(data & 0xFF) / 127.5f - 1; }
    [[nodiscard]] float y() const { return static_cast<float>(data >> 8 & 0xFF) / 127.5f - 1; }
    [[nodiscard]] float z() const { return static_cast<float>(data >> 16 & 0xFF) / 127.5f - 1; }
    [[nodiscard]] float w() const { return static_cast<float>(data >> 24 & 0xFF) / 127.5f - 1; }

  };
}
