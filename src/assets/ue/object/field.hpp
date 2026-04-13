//
// Created by oleub on 02.01.26.
//

#pragma once
#include "asset_object.hpp"

namespace SAA {
  class Field : public Object {
  public:
    Field(std::string_view className = "Field", uint64_t clsHash = StaticHASH) : Object(className, clsHash) {};

  protected:
    void parse() override;
  };

} 
