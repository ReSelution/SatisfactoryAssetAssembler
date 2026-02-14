//
// Created by oleub on 30.01.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGResourceDescriptor : public Object {
  public:
    FGResourceDescriptor(std::string_view className = "FGResourceDescriptor") : Object(className) {}

    uint32_t getClassHash() override { return HASH; };
    static constexpr uint32_t HASH = fnv1a("FGResourceDescriptor");
  };
  SAP_REGISTER_ASSET(FGResourceDescriptor)
} // namespace SAP
