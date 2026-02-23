//
// Created by oleub on 30.01.26.
//

#pragma once
#include "assets/satisfactory/ItemAmount.hpp"
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGSchematic : public Object {
    struct Schematic {
    // Identität & Fortschritt
    static constexpr uint32_t DisplayName                 = fnv1a("mDisplayName");
    static constexpr uint32_t Description                 = fnv1a("mDescription");
    static constexpr uint32_t TechTier                    = fnv1a("mTechTier");
    static constexpr uint32_t Type                        = fnv1a("mType");           // Enum: Milestone, MAM, Shop
    static constexpr uint32_t SchematicCategory           = fnv1a("mSchematicCategory");
    static constexpr uint32_t SubCategories               = fnv1a("mSubCategories");
    static constexpr uint32_t MenuPriority                = fnv1a("mMenuPriority");

    // Kosten & Freischaltung (Simplex-Input)
    static constexpr uint32_t Cost                        = fnv1a("mCost");           // Array von ItemAmounts
    static constexpr uint32_t Unlocks                     = fnv1a("mUnlocks");        // Enthält FGUnlockRecipe!
    static constexpr uint32_t TimeToComplete              = fnv1a("mTimeToComplete");

    // Logik & Abhängigkeiten (Dein Forschungsbaum)
    static constexpr uint32_t DependsOnSchematic          = fnv1a("mDependsOnSchematic");
    static constexpr uint32_t SchematicDependencies       = fnv1a("mSchematicDependencies");
    static constexpr uint32_t AdditionalDependencies      = fnv1a("mAdditionalSchematicDependencies");
    static constexpr uint32_t DependenciesBlocksAccess    = fnv1a("mDependenciesBlocksSchematicAccess");
    static constexpr uint32_t HiddenUntilDepsMet          = fnv1a("mHiddenUntilDependenciesMet");

    // UI & Meta
    static constexpr uint32_t SchematicIcon               = fnv1a("mSchematicIcon");
    static constexpr uint32_t SmallSchematicIcon          = fnv1a("mSmallSchematicIcon");
    static constexpr uint32_t RelevantShopSchematics      = fnv1a("mRelevantShopSchematics");
    static constexpr uint32_t RelevantEvents              = fnv1a("mRelevantEvents");
    static constexpr uint32_t StatisticGameplayTag        = fnv1a("mStatisticGameplayTag");
    static constexpr uint32_t IsPlayerSpecific            = fnv1a("mIsPlayerSpecific");
};
  public:
    FGSchematic(std::string_view classname = "FGSchematic") : Object(classname) {}
    uint32_t getClassHash() override { return HASH; };

    static constexpr uint32_t HASH = fnv1a("FGSchematic");
  protected:
    void parse() override;

  private:
    std::optional<FName> mType;
    std::optional<FText> mDisplayName;
    std::optional<FText> mDescription;
    std::optional<Object*> mSchematicIcon;
    std::optional<Object*> mSmallSchematicIcon;
    std::optional<Object*> mSchematicCategory;
    std::optional<Object*> mDependsOnSchematic;
    std::span<Object*> mUnlocks;
    std::span<Object*> mSchematicDependencies;
    std::span<Object*> mSubCategories;
    float mTimeToComplete;
    std::optional<Object*> mParentSchematic;
    std::span<Object*> mDependencies;
    std::span<ItemAmount> mCost;

    int32_t mTier;
    float mTimeToFinish;
    bool mHidden;




  };
  SAP_REGISTER_ASSET(FGSchematic)
} // namespace SAP
