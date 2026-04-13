//
// Created by oleub on 16.02.26.
//

#pragma once
#include "asset_object.hpp"
#include "object_factory.hpp"

namespace SAA {

  class RigVM:public Object {
    public:
    RigVM(std::string_view classname = "RigVM"):Object(classname){};

    void parse() override;
  };
  SAA_REGISTER_ASSET(RigVM)
} 
