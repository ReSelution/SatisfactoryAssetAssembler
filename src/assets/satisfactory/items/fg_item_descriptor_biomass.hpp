//
// Created by oleub on 30.01.26.
//

#pragma once

#include "fg_item_base.hpp"

#include "object_factory.hpp"

namespace SAA {
  using namespace sc::literals;

    class FGItemDescriptorBiomass : public FGItemBase {

    public:
        FGItemDescriptorBiomass(std::string_view classname = "FGItemDescriptorBiomass", uint64_t clsHash = StaticHASH) : FGItemBase(classname, clsHash) {}

        void parse() override;

#




        static constexpr uint64_t StaticHASH = "FGItemDescriptorBiomass"_h;
    protected:
        void exprt() override;

    };
    SAA_REGISTER_ASSET(FGItemDescriptorBiomass)
}
