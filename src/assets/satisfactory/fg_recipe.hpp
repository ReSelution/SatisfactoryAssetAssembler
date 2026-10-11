//
// Created by oleub on 20.01.26.
//

#pragma once
#include "assets/satisfactory/ItemAmount.hpp"
#include "asset_object.hpp"
#include "object_factory.hpp"


namespace SAA {
struct Recipe;

  class FGRecipe : public Object {
    friend class FGExporter;
    struct RecipeProps {
        static constexpr uint64_t ManufacturingMenuPriority      = "mManufacturingMenuPriority"_h;
        static constexpr uint64_t Product                        = "mProduct"_h;
        static constexpr uint64_t RelevantEvents                 = "mRelevantEvents"_h;
        static constexpr uint64_t DisplayName                    = "mDisplayName"_h;
        static constexpr uint64_t ProducedIn                     = "mProducedIn"_h;
        static constexpr uint64_t DisplayNameOverride            = "mDisplayNameOverride"_h;
        static constexpr uint64_t ManufactoringDuration          = "mManufactoringDuration"_h;
        static constexpr uint64_t VariablePowerConsumptionConstant = "mVariablePowerConsumptionConstant"_h;
        static constexpr uint64_t MaterialCustomizationRecipe    = "mMaterialCustomizationRecipe"_h;
        static constexpr uint64_t ManualManufacturingMultiplier  = "mManualManufacturingMultiplier"_h;
        static constexpr uint64_t OverriddenCategory             = "mOverriddenCategory"_h;
        static constexpr uint64_t VariablePowerConsumptionFactor  = "mVariablePowerConsumptionFactor"_h;
        static constexpr uint64_t Ingredients                    = "mIngredients"_h;
    };

  public:
    FGRecipe(std::string_view classname = "FGRecipe", uint64_t clsHash = StaticHASH) : Object(classname, clsHash) {
    }

      static constexpr uint64_t StaticHASH = "FGRecipe"_h;
  protected:
      void exprt() override;

    void parse() override;
  private:
    std::optional<float> mManufactoringDuration;
    std::optional<float> mManualManufacturingMultiplier;
    std::optional<float> mVariablePowerConsumptionFactor;
    std::optional<bool> mDisplayNameOverride;
    std::optional<float> mVariablePowerConsumptionConstant;

    std::optional<Object*> mOverriddenCategory;
    std::optional<Object*> mMaterialCustomizationRecipe;
    std::optional<sc::MagpieString> mDisplayName;
    std::span<SoftObj> mProducedIn;
    std::span<ItemAmount> mProduct;
    std::span<ItemAmount> mIngredients;
    Recipe* mRecipeExport = nullptr;
  };

  SAA_REGISTER_ASSET(FGRecipe)
}
