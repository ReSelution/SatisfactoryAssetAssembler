//
// Created by oleub on 27.01.26.
//

#pragma once
#include <cstdint>
#include <unordered_map>

#include "byte_cursor.hpp"

namespace SAP {
  struct FPerQualityLevelInt {
    bool cooked = false;
    int32_t defaultInt;
    std::unordered_map<int32_t, int32_t> perQualityLevelFloat;

    static FPerQualityLevelInt get(ByteCursor &cursor) {
      FPerQualityLevelInt result;
      result.cooked = cursor.readBool();
      result.defaultInt = cursor.read<int32_t>();
      auto count = cursor.read<int32_t>();
      result.perQualityLevelFloat.reserve(count);
      for (auto i = 0; i < count; i++) {
        result.perQualityLevelFloat[cursor.read<int32_t>()] = cursor.read<int32_t>();
      }
      return result;
    }
  };

  struct FPerQualityLevelFloat {
    bool cooked = false;
    float defaultFloat;
    std::unordered_map<int32_t, float> perQualityLevelFloat;

    static FPerQualityLevelFloat get(ByteCursor &cursor) {
      FPerQualityLevelFloat result;
      result.cooked = cursor.readBool();
      result.defaultFloat = cursor.read<float>();
      auto count = cursor.read<int32_t>();
      result.perQualityLevelFloat.reserve(count);
      for (auto i = 0; i < count; i++) {
        result.perQualityLevelFloat[cursor.read<int32_t>()] = cursor.read<float>();
      }
      return result;
    }
  };
} // namespace SAP
