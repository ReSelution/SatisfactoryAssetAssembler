//
// Created by oleub on 20.01.26.
//

#pragma once
#include "assets/satisfactory/ItemAmount.hpp"
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

struct Recipe;

namespace SAP {
  class FGRecipe : public Object {
    friend class FGExporter;
    struct RecipeProps {
      static constexpr uint32_t ManufacturingMenuPriority = fnv1a("mManufacturingMenuPriority");
      static constexpr uint32_t Product = fnv1a("mProduct");
      static constexpr uint32_t RelevantEvents = fnv1a("mRelevantEvents");
      static constexpr uint32_t DisplayName = fnv1a("mDisplayName");
      static constexpr uint32_t ProducedIn = fnv1a("mProducedIn");
      static constexpr uint32_t DisplayNameOverride = fnv1a("mDisplayNameOverride");
      static constexpr uint32_t ManufactoringDuration = fnv1a("mManufactoringDuration");
      static constexpr uint32_t VariablePowerConsumptionConstant = fnv1a("mVariablePowerConsumptionConstant");
      static constexpr uint32_t MaterialCustomizationRecipe = fnv1a("mMaterialCustomizationRecipe");
      static constexpr uint32_t ManualManufacturingMultiplier = fnv1a("mManualManufacturingMultiplier");
      static constexpr uint32_t OverriddenCategory = fnv1a("mOverriddenCategory");
      static constexpr uint32_t VariablePowerConsumptionFactor = fnv1a("mVariablePowerConsumptionFactor");
      static constexpr uint32_t Ingredients = fnv1a("mIngredients");
    };

  public:
    FGRecipe(std::string_view classname = "FGRecipe") : Object(classname) {
    }

    uint32_t getClassHash() override { return HASH; }
  protected:
    void exprt() override;
    void parse() override;

  private:
    static constexpr uint32_t HASH = fnv1a("FGRecipe");
    std::optional<float> mManufactoringDuration;
    std::optional<float> mManualManufacturingMultiplier;
    std::optional<float> mVariablePowerConsumptionFactor;
    std::optional<bool> mDisplayNameOverride;
    std::optional<float> mVariablePowerConsumptionConstant;

    std::optional<Object*> mOverriddenCategory;
    std::optional<Object*> mMaterialCustomizationRecipe;
    std::optional<FText> mDisplayName;
    std::span<SoftObj> mProducedIn;
    std::span<ItemAmount> mProduct;
    std::span<ItemAmount> mIngredients;
    Recipe* mRecipeExport = nullptr;
  };

  SAP_REGISTER_ASSET(FGRecipe)
} // namespace SAP
