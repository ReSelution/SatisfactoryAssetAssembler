//
// Created by oleub on 30.01.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGSchematic: public Object {
    public:
    FGSchematic(std::string_view classname = "FGSchematic" ): Object(classname) {}
    uint32_t getClassHash() override {
      return HASH;
    };

    static constexpr uint32_t HASH = fnv1a("FGSchematic");
  };
  SAP_REGISTER_ASSET(FGSchematic)
} // SAP