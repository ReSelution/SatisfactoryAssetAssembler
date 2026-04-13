//
// Created by oleub on 24.02.26.
//

#pragma once
#include <cstdint>
#include <optional>

#include "core/f_text.hpp"
#include "asset_object.hpp"


namespace SAA {
  struct ItemAmount;
  struct SoftObj;


  enum class ExportType : uint32_t {
    Unknown = 0,
    Recipe,
    ItemDescriptor,
    Building,
    Schematic,
    COUNT // Help for Array Size
  };

  struct MetaExport {
    static constexpr auto Kind = ExportType::Unknown;
  };

  struct Recipe : MetaExport {
    static constexpr auto Kind = ExportType::Recipe;

    std::optional<float> mManufactoringDuration;
    std::optional<float> mManualManufacturingMultiplier;
    std::optional<float> mVariablePowerConsumptionFactor;
    std::optional<bool> mDisplayNameOverride;
    std::optional<float> mVariablePowerConsumptionConstant;

    std::optional<FText> mDisplayName;
    std::span<SoftObj> mProducedIn;
    std::span<ItemAmount> mProduct;
    std::span<ItemAmount> mIngredients;
  };

  struct ItemDescriptor : MetaExport {
    static constexpr auto Kind = ExportType::ItemDescriptor;

    // Basic Info
    std::optional<FText> mDisplayName;
    std::optional<FText> mDescription;
    std::optional<FText> mAbbreviatedDisplayName;

    // Icons & Visuals
    std::optional<Object *> mCategory;
    std::optional<Object *> mPersistentBigIcon;
    std::optional<Object *> mSmallIcon;
    std::optional<Object *> mConveyorMesh;
    std::optional<Object *> mCompassTexture;
    std::optional<Object *> mDepositMesh;

    // Physical Properties
    std::optional<FName> mStackSize;
    std::optional<FName> mForm;
    std::optional<FColor> mFluidColor;
    std::optional<FColor> mGasColor;
    std::optional<FLinearColor> mPingColor;

    // Stats
    float mEnergyValue{0.0f};
    float mRadioactiveDecay{0.0f};
    bool mCanBeDiscarded{false};
    bool mIsAlienItem{false};
    int32_t mItemIndex{0};

    // Scanner & Logic
    std::optional<FName> mScannableType;
    std::optional<Object *> mClassToScanFor;
    std::optional<FText> mScannerDisplayText;
    std::optional<Object *> mRequiredSchematic;
  };

  struct Building : MetaExport {
    static constexpr auto Kind = ExportType::Building;

    std::optional<FText> mDisplayName;
    std::optional<FText> mDescription;

    std::optional<Object *> mCategory;
    std::optional<Object *> mPersistentBigIcon;
    std::optional<Object *> mSmallIcon;
    std::span<Object *> mSubCategories;
    std::optional<Object *> mConveyorMesh;
    std::optional<FName> mStackSize;

    Object *mPowerInfo = nullptr;

    int32_t mInventorySizeY = 0;
    int32_t mInventorySizeX = 0;

    float mPowerConsumption = 0.0f;
    float mPowerConsumptionExp = 0.0f;
    float mPowerProduction = 0.0f;
    float mVariablePowerProductionConstant = 0.0f;
    Object *mPowerConsumptionCurve = nullptr;

    float mProductionShardBoostMultiplier = 0.0f;
    int32_t mProductionShardSlotSize = 1;

    std::span<Object *> mAllowedResources;
    std::span<FName> mAllowedResourceForms;
  };
}
