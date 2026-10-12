//
// Created by oleub on 27.01.26.
//

#pragma once
#include <cstdint>
#include <span>

#include "byte_cursor.hpp"

namespace SAA {
  // One value per quality level, as it is stored. The entries stay in the file buffer: a property value must not
  // own memory outside the storage of its file (see f_property_tag.hpp).
  template<typename T>
  struct FPerQualityLevelEntry {
    int32_t qualityLevel;
    T value;
  };

  struct FPerQualityLevelInt {
    bool cooked = false;
    int32_t defaultInt;
    std::span<const FPerQualityLevelEntry<int32_t>> perQualityLevel;

    static FPerQualityLevelInt get(ByteCursor &cursor) {
      FPerQualityLevelInt result;
      result.cooked          = cursor.readBool();
      result.defaultInt      = cursor.read<int32_t>();
      result.perQualityLevel = cursor.readArray<FPerQualityLevelEntry<int32_t>>();
      return result;
    }
  };

  struct FPerQualityLevelFloat {
    bool cooked = false;
    float defaultFloat;
    std::span<const FPerQualityLevelEntry<float>> perQualityLevel;

    static FPerQualityLevelFloat get(ByteCursor &cursor) {
      FPerQualityLevelFloat result;
      result.cooked          = cursor.readBool();
      result.defaultFloat    = cursor.read<float>();
      result.perQualityLevel = cursor.readArray<FPerQualityLevelEntry<float>>();
      return result;
    }
  };
} // namespace SAA
