//
// Created by oleub on 20.01.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGRecipe : public Object {
  public:
    FGRecipe(std::string_view classname = "FGRecipe") : Object(classname) {}
    uint32_t getClassHash() override {
      return HASH;
    }
    void parse() override;
  private:
    static constexpr uint32_t HASH = fnv1a("FGRecipe");
  };

  SAP_REGISTER_ASSET(FGRecipe)
}
