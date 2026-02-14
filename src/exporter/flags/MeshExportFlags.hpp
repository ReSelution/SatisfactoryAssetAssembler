//
// Created by oleub on 10.01.26.
//

#pragma once
#include <cstdint>

enum class MeshExportFlags : uint32_t {
  None = 0,
  GLTF2 = 1 << 0,
};

// Bitwise Operatoren für TextureExportFlags
constexpr MeshExportFlags operator|(MeshExportFlags a, MeshExportFlags b) {
  return static_cast<MeshExportFlags>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr MeshExportFlags operator&(MeshExportFlags a, MeshExportFlags b) {
  return static_cast<MeshExportFlags>(static_cast<int>(a) & static_cast<int>(b));
}

constexpr MeshExportFlags operator~(MeshExportFlags a) { return static_cast<MeshExportFlags>(~static_cast<int>(a)); }

// Kombinierte Operatoren für Assignment
inline MeshExportFlags &operator|=(MeshExportFlags &a, MeshExportFlags b) {
  a = a | b;
  return a;
}

inline MeshExportFlags &operator&=(MeshExportFlags &a, MeshExportFlags b) {
  a = a & b;
  return a;
}
