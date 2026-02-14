//
// Created by oleub on 02.01.26.
//

#pragma once
#include "ue/asset_object.hpp"

namespace SAP {
  class Field : public Object {
  public:
    Field(std::string_view className = "Field") : Object(className) {};

  protected:
    void parse() override;
  };

} // namespace SAP
