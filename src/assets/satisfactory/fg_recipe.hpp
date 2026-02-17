//
// Created by oleub on 20.01.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGRecipe : public Object {
    using hash_t = uint32_t;

    struct Recipe {
      static constexpr hash_t ManufacturingMenuPriority        = fnv1a("mManufacturingMenuPriority");
      static constexpr hash_t Product                          = fnv1a("mProduct");
      static constexpr hash_t RelevantEvents                   = fnv1a("mRelevantEvents");
      static constexpr hash_t DisplayName                      = fnv1a("mDisplayName");
      static constexpr hash_t ProducedIn                       = fnv1a("mProducedIn");
      static constexpr hash_t DisplayNameOverride              = fnv1a("mDisplayNameOverride");
      static constexpr hash_t ManufactoringDuration            = fnv1a("mManufactoringDuration");
      static constexpr hash_t VariablePowerConsumptionConstant = fnv1a("mVariablePowerConsumptionConstant");
      static constexpr hash_t MaterialCustomizationRecipe      = fnv1a("mMaterialCustomizationRecipe");
      static constexpr hash_t ManualManufacturingMultiplier    = fnv1a("mManualManufacturingMultiplier");
      static constexpr hash_t OverriddenCategory               = fnv1a("mOverriddenCategory");
      static constexpr hash_t VariablePowerConsumptionFactor   = fnv1a("mVariablePowerConsumptionFactor");
      static constexpr hash_t Ingredients                      = fnv1a("mIngredients");
    };

  public:
    FGRecipe(std::string_view classname = "FGRecipe") : Object(classname) {}
    uint32_t getClassHash() override { return HASH; }
    void parse() override;

  private:
    static constexpr uint32_t HASH = fnv1a("FGRecipe");
  };

  SAP_REGISTER_ASSET(FGRecipe)
} // namespace SAP
