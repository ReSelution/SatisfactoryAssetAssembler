//
// Created by oleub on 24.02.26.
//

#include "fg_exporter.hpp"


#include "satisfactory/fg_recipe.hpp"
#include "satisfactory/building/fg_building_base.hpp"
#include "satisfactory/items/fg_item_base.hpp"


namespace SAA {
    void FGExporter::encodeRecipe(FGRecipe *recipe) {
        auto e = getEntity(recipe);
      // if (recipe->mDisplayName.has_value()) {
      //   exLog::info("Exporting Recipe: {}", recipe->mDisplayName.value().view());
      // }
//    auto ex = Registry::write<Recipe>(
//      recipe->mManufactoringDuration,
//      recipe->mManualManufacturingMultiplier,
//      recipe->mVariablePowerConsumptionFactor,
//      recipe->mDisplayNameOverride,
//      recipe->mVariablePowerConsumptionConstant,
//      recipe->mDisplayName,
//      recipe->mProducedIn,
//      recipe->mProduct
//    );

//    for (auto &ing: recipe->mIngredients) {
//      auto obj = FGFactory::createBase(ing.object);
//      obj->safeParse();
//      obj->addUsedIn(ex);
//    }
//    for (auto &pro: recipe->mProduct) {
//      auto obj = FGFactory::createBase(pro.object);
//      obj->safeParse();
//      obj->addProducedIn(ex);
//    }
//
    }


    void FGExporter::encodeItem(FGItemBase *descriptor) {
        auto e = getEntity(descriptor);
//        Object *scanFor = nullptr;
//    if (descriptor->mClassToScanFor.has_value()) {
//      scanFor = descriptor->mClassToScanFor->resolve(descriptor->m_owner->assetParser());
//    }
//    auto ex = Registry::write<ItemDescriptor>(
//      descriptor->mDisplayName,
//      descriptor->mDescription,
//      descriptor->mAbbreviatedDisplayName,
//      descriptor->mCategory,
//      descriptor->mPersistentBigIcon,
//      descriptor->mSmallIcon,
//      descriptor->mConveyorMesh,
//      descriptor->mCompassTexture,
//      descriptor->mDepositMesh,
//      descriptor->mStackSize,
//      descriptor->mForm,
//      descriptor->mFluidColor,
//      descriptor->mGasColor,
//      descriptor->mPingColor,
//      descriptor->mEnergyValue,
//      descriptor->mRadioactiveDecay,
//      descriptor->mCanBeDiscarded,
//      descriptor->mIsAlienItem,
//      descriptor->mItemIndex,
//      descriptor->mScannableType,
//      scanFor,
//      descriptor->mScannerDisplayText,
//      descriptor->mRequiredSchematic
//    );
    }

    void FGExporter::encodeBuilding(FGBuildingBase *buildingBase) {
        auto e = getEntity(buildingBase);
//        auto building = FGFactory::createBuilding(buildingBase->mBuildableClass);
//        building->safeParse();
//        auto ex = Registry::write<Building>(
//                building->mDisplayName,
//                building->mDescription,
//                buildingBase->mCategory,
//                buildingBase->mPersistentBigIcon,
//                buildingBase->mSmallIcon,
//                buildingBase->mSubCategories,
//                buildingBase->mConveyorMesh,
//                buildingBase->mStackSize,
//                building->mPowerInfo,
//                building->mInventorySizeX,
//                building->mInventorySizeY,
//                building->mPowerConsumption,
//                building->mPowerConsumptionExp,
//                building->mPowerProduction,
//                building->mVariablePowerProductionConstant,
//                building->mPowerConsumptionCurve,
//                building->mProductionShardBoostMultiplier,
//                building->mProductionShardSlotSize,
//                building->mAllowedResources,
//                building->mAllowedResourceForms
//        );
    }
}
