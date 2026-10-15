//
// Created by oleub on 21.02.26.
//

#pragma once
#include <cstdint>
#include "asset_object.hpp"

namespace SAA {
  using namespace sc::literals;


struct ItemAmount {
    static constexpr uint64_t Amount = "Amount"_h;
    static constexpr uint64_t ItemClass = "ItemClass"_h;
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
