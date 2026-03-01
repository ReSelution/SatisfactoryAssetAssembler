//
// Created by oleub on 24.02.26.
//

#include "fg_exporter.hpp"

#include "puplic/arena/registery.hpp"
#include "satisfactory/fg_recipe.hpp"
#include "ue/properties/soft_object.hpp"


namespace SAP {
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
} // SAP
