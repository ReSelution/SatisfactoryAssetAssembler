//
// Created by oleub on 30.01.26.
//

#pragma once
#include "simdjson.h"

#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGItemDescriptorBiomass: public Object {
    public:
    FGItemDescriptorBiomass(std::string_view classname = "FGItemDescriptorBiomass"): Object(classname) { }
#
    uint32_t getClassHash() override {
      return HASH;
    };

    static constexpr uint32_t HASH = fnv1a("FGItemDescriptorBiomass");
  };
  SAP_REGISTER_ASSET(FGItemDescriptorBiomass)
} // SAP