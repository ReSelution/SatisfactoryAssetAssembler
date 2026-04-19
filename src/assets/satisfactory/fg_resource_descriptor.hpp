//
// Created by oleub on 30.01.26.
//

#pragma once
#include "items/fg_item_base.hpp"
#include "object_factory.hpp"

namespace SAA {
  class FGResourceDescriptor : public FGItemBase {
    struct Resource {

    };

  public:
    explicit FGResourceDescriptor(const std::string_view className = "FGResourceDescriptor", uint64_t clsHash = StaticHASH) : FGItemBase(className, clsHash) {
    }


    static constexpr uint64_t StaticHASH = "FGResourceDescriptor"_h;

  protected:
    void parse() override;

    void exprt() override;
  };

  SAA_REGISTER_ASSET(FGResourceDescriptor)
} 
