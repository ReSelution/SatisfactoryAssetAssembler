//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <string>

constexpr uint32_t fnv1a(const char* str, uint32_t hash = 2166136261u)
{
  return *str ? fnv1a(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619u) : hash;
}

template <typename T>
  T hashEnum(const std::string &value) {
  static_assert(std::is_enum<T>::value, "T must be an enum");
  return static_cast<T>(fnv1a(value.c_str()));
}
