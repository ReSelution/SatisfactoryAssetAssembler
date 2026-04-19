//
// Created by oleub on 24.02.26.
//

#pragma once

#include "exporter.hpp"

namespace SAA {
    struct Building;
    class FGBuildingBase;
    class FGItemDescriptor;
    class FGRecipe;
    class FGItemBase;

    struct ItemDescriptor;
    struct Recipe;

    class FGExporter : public Exporter {
    public:

        static void encodeRecipe(FGRecipe *recipe);
        static void encodeItem(FGItemBase *item);
        static void encodeBuilding(FGBuildingBase *buildingBase);
    };
} // namespace SAA
