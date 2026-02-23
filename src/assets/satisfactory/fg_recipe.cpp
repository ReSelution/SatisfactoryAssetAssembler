//
// Created by oleub on 20.01.26.
//
#include "fg_recipe.hpp"

#include "PropertyTracker.hpp"

namespace SAP {
  void FGRecipe::parse() {
    Object::parse();
    DBG_TRACK_MAP(m_className, m_properties);
    DBG_MARK_HANDLED(m_className, Recipe::ManufacturingMenuPriority);
    DBG_MARK_HANDLED(m_className, Recipe::RelevantEvents);

    mProducedIn = getArray<SoftObj>(Recipe::ProducedIn);
    if (mProducedIn.empty()) { return; }

    mManufactoringDuration = get<float>(Recipe::ManufactoringDuration);
    mVariablePowerConsumptionConstant = get<float>(Recipe::VariablePowerConsumptionConstant);
    mManualManufacturingMultiplier = get<float>(Recipe::ManualManufacturingMultiplier);
    mVariablePowerConsumptionFactor = get<float>(Recipe::VariablePowerConsumptionFactor);
    mDisplayName = get<FText>(Recipe::DisplayName);
    mProduct = getArray<ItemAmount>(Recipe::Product, &ItemAmount::get);
    mDisplayNameOverride = get<bool>(Recipe::DisplayNameOverride);
    mOverriddenCategory = get<Object *>(Recipe::OverriddenCategory);
    mMaterialCustomizationRecipe = get<Object *>(Recipe::MaterialCustomizationRecipe);
    mIngredients = getArray<ItemAmount>(Recipe::Ingredients, &ItemAmount::get);

  }
} // namespace SAP
