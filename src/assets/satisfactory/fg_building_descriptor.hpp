//
// Created by oleub on 30.01.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGBuildingDescriptor: public Object {
    public:
    FGBuildingDescriptor(std::string classname = "FGBuildingDescriptor"): Object(classname) {}

    uint32_t getClassHash() override {
      return HASH;
    };

    void parse() override;

    static constexpr uint32_t HASH = fnv1a("FGBuildingDescriptor");
  };

  SAP_REGISTER_ASSET(FGBuildingDescriptor)
} // SAP