//
// Created by oleub on 20.01.26.
//
#include "fg_recipe.hpp"

#include "PropertyTracker.hpp"
#include "fg/fg_exporter.hpp"

namespace SAA {
  void FGRecipe::exprt() {
    FGExporter::encodeRecipe(this);
  }

  void FGRecipe::parse() {
    Object::parse();
    DBG_TRACK_MAP(m_className, m_properties);
    DBG_MARK_HANDLED(m_className, RecipeProps::ManufacturingMenuPriority);
    DBG_MARK_HANDLED(m_className, RecipeProps::RelevantEvents);

    mProducedIn = getArray<SoftObj>(RecipeProps::ProducedIn);
    if (mProducedIn.empty()) { return; }

    mManufactoringDuration = get<float>(RecipeProps::ManufactoringDuration);
    mVariablePowerConsumptionConstant = get<float>(RecipeProps::VariablePowerConsumptionConstant);
    mManualManufacturingMultiplier = get<float>(RecipeProps::ManualManufacturingMultiplier);
    mVariablePowerConsumptionFactor = get<float>(RecipeProps::VariablePowerConsumptionFactor);
    mDisplayName = get<SC::MagpieString>(RecipeProps::DisplayName);
    mProduct = getArray<ItemAmount>(RecipeProps::Product, &ItemAmount::get);
    mDisplayNameOverride = get<bool>(RecipeProps::DisplayNameOverride);
    mOverriddenCategory = get<Object *>(RecipeProps::OverriddenCategory);
    mMaterialCustomizationRecipe = get<Object *>(RecipeProps::MaterialCustomizationRecipe);
    mIngredients = getArray<ItemAmount>(RecipeProps::Ingredients, &ItemAmount::get);

  }
} 
