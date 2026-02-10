//
// Created by oleub on 30.01.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGItemDescriptor : public Object {
  public:
    FGItemDescriptor(std::string_view classname = "FGItemDescriptor") : Object(classname) {};

    uint32_t getClassHash() override {
      return HASH;
    };
    void parse() override;

    static constexpr uint32_t HASH = fnv1a("FGItemDescriptor");

  };

  SAP_REGISTER_ASSET(FGItemDescriptor)
} // SAP
