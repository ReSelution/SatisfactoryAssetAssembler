//
// Created by oleub on 21.02.26.
//

#pragma once
#include <cstdint>

#include "ue/asset_object.hpp"

namespace SAP {
  struct ItemAmount {
    static constexpr uint32_t Amount = fnv1a("Amount");
    static constexpr uint32_t ItemClass = fnv1a("ItemClass");
    int32_t amount;
    Object *object;

    static ItemAmount get(const PropertyHolder &holder) {
      return {
        . amount = holder.get<int32_t>(Amount).value(),
        . object = holder.get<Object *>(ItemClass).value(),
      };
    }
  };
}
