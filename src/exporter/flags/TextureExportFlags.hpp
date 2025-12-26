//
// Created by oleub on 24.12.25.
//

#pragma once
#include <cstdint>

enum class TextureExportFlags : uint32_t {
  None = 0,
  PNG  = 1 << 0,
};

// Bitwise Operatoren für TextureExportFlags
inline constexpr TextureExportFlags operator|(TextureExportFlags a, TextureExportFlags b) {
  return static_cast<TextureExportFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline constexpr TextureExportFlags operator&(TextureExportFlags a, TextureExportFlags b) {
  return static_cast<TextureExportFlags>(static_cast<int>(a) & static_cast<int>(b));
}

inline constexpr TextureExportFlags operator~(TextureExportFlags a) {
  return static_cast<TextureExportFlags>(~static_cast<int>(a));
}

// Kombinierte Operatoren für Assignment
inline TextureExportFlags& operator|=(TextureExportFlags& a, TextureExportFlags b) {
  a = a | b;
  return a;
}

inline TextureExportFlags& operator&=(TextureExportFlags& a, TextureExportFlags b) {
  a = a & b;
  return a;
}