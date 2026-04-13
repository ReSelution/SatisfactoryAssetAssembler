//
// Created by oleub on 30.01.26.
//

#pragma once
#include "fg_item_base.hpp"
#include "asset_object.hpp"
#include "object_factory.hpp"

namespace SAA {
  class FGItemDescriptor : public FGItemBase {
    friend class FGExporter;

  public:
    explicit FGItemDescriptor(std::string_view classname = "FGItemDescriptor", uint64_t clsHash = StaticHASH) : FGItemBase(classname, clsHash) {
    };


    void parse() override;

    static constexpr uint64_t StaticHASH = "FGItemDescriptor"_h;
  protected:
    void exprt() override;

  };

  SAA_REGISTER_ASSET(FGItemDescriptor)
}
