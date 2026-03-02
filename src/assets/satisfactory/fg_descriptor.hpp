//
// Created by oleub on 01.03.26.
//

#pragma once
#include "ue/asset_object.hpp"

namespace SAA {
  class FGDescriptor:public Object{
    public:
    explicit FGDescriptor(const std::string_view className = "FGDescriptor"): Object(className) {

    }
  };
} // SAP