//
// Created by oleub on 30.01.26.
//

#pragma once

#include "fg_item_base.hpp"
#include "simdjson.h"

#include "asset_object.hpp"
#include "object_factory.hpp"

namespace SAA {
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
