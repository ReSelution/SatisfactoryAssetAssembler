//
// Created by oleub on 24.02.26.
//

#pragma once
#include <cstdint>
#include <optional>

#include "core/f_text.hpp"
#include "ue/asset_object.hpp"


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
  std::optional<FText> mDisplayName;
  std::optional<FText> mDescription;
  std::optional<FText> mAbbreviatedDisplayName;

  std::optional<Object *> mCategory;
  std::optional<Object *> mPersistentBigIcon;
  std::optional<Object *> mSmallIcon;
  std::optional<Object *> mConveyorMesh;

  std::optional<FName> mStackSize;
  std::optional<FName> mForm;
  std::optional<FColor> mFluidColor;
  std::optional<FColor> mGasColor;

  float mEnergyValue{};
  float mRadioactiveDecay{};
  bool mCanBeDiscarded{};
  bool mIsAlienItem{};
  int32_t mItemIndex{};

  std::optional<FName> mScannableType;
  std::optional<SoftObj> mClassToScanFor;
  std::optional<FText> mScannerDisplayText;
  std::optional<Object *> mRequiredSchematic;
};
}