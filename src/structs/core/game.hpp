//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

constexpr uint32_t GameUe4Base = 0x04000000;
constexpr uint32_t GameUe5Base = 0x05000000;

enum class EGame : uint32_t {
  UE4_LATEST = GameUe4Base + (28 << 16),

  // UE5 Haupt-Releases
  UE5_0 = GameUe5Base + (0 << 16),
  UE5_1 = GameUe5Base + (1 << 16),
  UE5_2 = GameUe5Base + (2 << 16),
  UE5_3 = GameUe5Base + (3 << 16),
  UE5_4 = GameUe5Base + (4 << 16),
  UE5_5 = GameUe5Base + (5 << 16),
  UE5_6 = GameUe5Base + (6 << 16),
  UE5_7 = GameUe5Base + (7 << 16),
  UE5_8 = GameUe5Base + (8 << 16),

  UE5_LATEST = UE5_8
};

inline bool operator>=(uint32_t lhs, EGame rhs) { return lhs >= static_cast<uint32_t>(rhs); }
