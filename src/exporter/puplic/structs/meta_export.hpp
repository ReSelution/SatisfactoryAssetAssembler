//
// Created by oleub on 24.02.26.
//

#pragma once
#include <cstdint>
#include <optional>

#include "core/f_text.hpp"

namespace SAP {
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

  std::optional<SAP::FText> mDisplayName;
  std::span<SAP::SoftObj> mProducedIn;
  std::span<SAP::ItemAmount> mProduct;
  std::span<SAP::ItemAmount> mIngredients;
};