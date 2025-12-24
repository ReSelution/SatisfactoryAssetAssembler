//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

enum class EUsmapVersion : uint8_t {
  /* Initial format. */
  INITIAL = 0,
  PACKAGE_VERSIONING = 1,
  LONG_F_NAME = 2,
  LARGE_ENUMS = 3,
  EXPLICIT_ENUM_VALUES = 4,
  LATEST_PLUS_ONE = 5,
  LATEST = LATEST_PLUS_ONE - 1
};


enum class EUsmapCompressionMethod : uint8_t { NONE = 0, OODLE = 1, Z_STANDARD = 2, UNKNOWN = 0xFF };