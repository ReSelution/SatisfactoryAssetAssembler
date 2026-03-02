//
// Created by oleub on 24.02.26.
//

#include "fg_exporter.hpp"

#include "puplic/arena/registery.hpp"
#include "satisfactory/fg_item_descriptor.hpp"
#include "satisfactory/fg_recipe.hpp"
#include "ue/properties/soft_object.hpp"


namespace SAA {
  Recipe *FGExporter::encode(FGRecipe *recipe) {
    auto ex = Registry::write<Recipe>(
      recipe->mManufactoringDuration,
      recipe->mManualManufacturingMultiplier,
      recipe->mVariablePowerConsumptionFactor,
      recipe->mDisplayNameOverride,
      recipe->mVariablePowerConsumptionConstant,
      recipe->mDisplayName,
      recipe->mProducedIn,
      recipe->mProduct

    );
    return ex;
  }

  ItemDescriptor * FGExporter::encode(FGItemDescriptor *descriptor) {
    auto ex = Registry::write<ItemDescriptor>(
        descriptor->mDisplayName,
        descriptor->mDescription,
        descriptor->mAbbreviatedDisplayName,
        descriptor->mCategory,
        descriptor->mPersistentBigIcon,
        descriptor->mSmallIcon,
        descriptor->mConveyorMesh,
        descriptor->mStackSize,
        descriptor->mForm,
        descriptor->mFluidColor,
        descriptor->mGasColor,
        descriptor->mEnergyValue,
        descriptor->mRadioactiveDecay,
        descriptor->mCanBeDiscarded,
        descriptor->mIsAlienItem,
        descriptor->mItemIndex,
        descriptor->mScannableType,
        descriptor->mClassToScanFor,
        descriptor->mScannerDisplayText,
        descriptor->mRequiredSchematic
      );
    return ex;
  }
} // SAP
