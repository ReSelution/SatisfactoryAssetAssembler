//
// Created by oleub on 30.01.26.
//

#pragma once
#include "assets/satisfactory/ItemAmount.hpp"
#include "asset_object.hpp"
#include "object_factory.hpp"

namespace SAA {
  class FGSchematic : public Object {
    struct Schematic {
        static constexpr uint64_t DisplayName                = "mDisplayName"_h;
        static constexpr uint64_t Description                = "mDescription"_h;
        static constexpr uint64_t TechTier                   = "mTechTier"_h;
        static constexpr uint64_t Type                       = "mType"_h;           // Enum: Milestone, MAM, Shop
        static constexpr uint64_t SchematicCategory          = "mSchematicCategory"_h;
        static constexpr uint64_t SubCategories              = "mSubCategories"_h;
        static constexpr uint64_t MenuPriority               = "mMenuPriority"_h;

        // Kosten & Freischaltung (Simplex-Input)
        static constexpr uint64_t Cost                       = "mCost"_h;           // Array von ItemAmounts
        static constexpr uint64_t Unlocks                    = "mUnlocks"_h;        // Enthält FGUnlockRecipe!
        static constexpr uint64_t TimeToComplete             = "mTimeToComplete"_h;

        // Logik & Abhängigkeiten (Dein Forschungsbaum)
        static constexpr uint64_t DependsOnSchematic         = "mDependsOnSchematic"_h;
        static constexpr uint64_t SchematicDependencies      = "mSchematicDependencies"_h;
        static constexpr uint64_t AdditionalDependencies     = "mAdditionalSchematicDependencies"_h;
        static constexpr uint64_t DependenciesBlocksAccess   = "mDependenciesBlocksSchematicAccess"_h;
        static constexpr uint64_t HiddenUntilDepsMet         = "mHiddenUntilDependenciesMet"_h;
        static constexpr uint64_t SchematicUnlockTag         = "mSchematicUnlockTag"_h;

        // UI & Meta
        static constexpr uint64_t SchematicIcon              = "mSchematicIcon"_h;
        static constexpr uint64_t SmallSchematicIcon         = "mSmallSchematicIcon"_h;
        static constexpr uint64_t RelevantShopSchematics     = "mRelevantShopSchematics"_h;
        static constexpr uint64_t RelevantEvents             = "mRelevantEvents"_h;
        static constexpr uint64_t StatisticGameplayTag       = "mStatisticGameplayTag"_h;
        static constexpr uint64_t IsPlayerSpecific           = "mIsPlayerSpecific"_h;
};
  public:
    FGSchematic(std::string_view classname = "FGSchematic", uint64_t clsHash = StaticHASH) : Object(classname, clsHash) {}

    static constexpr uint64_t StaticHASH = "FGSchematic"_h;
  protected:
    void parse() override;

  private:
    std::optional<FName> mType;
    std::optional<sc::MagpieString> mDisplayName;
    std::optional<sc::MagpieString> mDescription;
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
  SAA_REGISTER_ASSET(FGSchematic)
} 
