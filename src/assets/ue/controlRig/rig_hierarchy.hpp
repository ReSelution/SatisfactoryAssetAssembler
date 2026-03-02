//
// Created by oleub on 15.02.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAA {

  class RigHierarchy: public Object {
    public:
    RigHierarchy(std::string className= "RigHierarchy"): Object(className) {};

    void parse() override;
  };
  SAA_REGISTER_ASSET(RigHierarchy)
} // namespace SAP
