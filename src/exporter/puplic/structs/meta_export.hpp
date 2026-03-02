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
}

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

  std::optional<SAA::FText> mDisplayName;
  std::span<SAA::SoftObj> mProducedIn;
  std::span<SAA::ItemAmount> mProduct;
  std::span<SAA::ItemAmount> mIngredients;
};


struct ItemDescriptor : MetaExport {
  static constexpr auto Kind = ExportType::ItemDescriptor;
  std::optional<SAA::FText> mDisplayName;
  std::optional<SAA::FText> mDescription;
  std::optional<SAA::FText> mAbbreviatedDisplayName;

  std::optional<SAA::Object *> mCategory;
  std::optional<SAA::Object *> mPersistentBigIcon;
  std::optional<SAA::Object *> mSmallIcon;
  std::optional<SAA::Object *> mConveyorMesh;

  std::optional<SAA::FName> mStackSize;
  std::optional<SAA::FName> mForm;
  std::optional<SAA::FColor> mFluidColor;
  std::optional<SAA::FColor> mGasColor;

  float mEnergyValue{};
  float mRadioactiveDecay{};
  bool mCanBeDiscarded{};
  bool mIsAlienItem{};
  int32_t mItemIndex{};

  std::optional<SAA::FName> mScannableType;
  std::optional<SAA::SoftObj> mClassToScanFor;
  std::optional<SAA::FText> mScannerDisplayText;
  std::optional<SAA::Object *> mRequiredSchematic;
};
