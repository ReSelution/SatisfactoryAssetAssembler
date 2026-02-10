//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <string>

constexpr uint32_t fnv1a(std::string_view str) {
  uint32_t hash = 2166136261u;
  for (char c : str) {
    hash ^= static_cast<uint8_t>(c);
    hash *= 16777619u;
  }
  return hash;
}
template <typename T>
  T hashEnum( std::string_view value) {
  static_assert(std::is_enum<T>::value, "T must be an enum");
  return static_cast<T>(fnv1a(value));
}
