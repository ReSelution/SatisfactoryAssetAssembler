//
// Created by oleub on 30.01.26.
//

#pragma once
#include "fg_building_base.hpp"
#include "asset_object.hpp"
#include "object_factory.hpp"

namespace SAA {
  class FGBuildingDescriptor : public FGBuildingBase {

  public:
    FGBuildingDescriptor(const std::string& classname = "FGBuildingDescriptor",uint64_t clsHash = StaticHASH) : FGBuildingBase(classname, clsHash) {}


    void parse() override;

    static constexpr uint64_t StaticHASH = "FGBuildingDescriptor"_h;

  protected:
    void exprt() override;


  };

  SAA_REGISTER_ASSET(FGBuildingDescriptor)
} 
