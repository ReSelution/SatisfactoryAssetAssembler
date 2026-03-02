//
// Created by oleub on 30.01.26.
//

#include "fg_schematic.hpp"

namespace SAA {
  void FGSchematic::parse() {
    Object::parse();
    DBG_MARK_HANDLED(m_className, Schematic::MenuPriority);
    DBG_MARK_HANDLED(m_className, Schematic::RelevantEvents);
    DBG_MARK_HANDLED(m_className, Schematic::StatisticGameplayTag);
    DBG_MARK_HANDLED(m_className, Schematic::AdditionalDependencies);
    DBG_MARK_HANDLED(m_className, Schematic::RelevantShopSchematics);
    DBG_MARK_HANDLED(m_className, Schematic::IsPlayerSpecific);

    mDisplayName = get<FText>(Schematic::DisplayName);
    mDescription = get<FText>(Schematic::Description);

    mTier = get<int32_t>(Schematic::TechTier).value_or(0);
    mType = get<FName>(Schematic::Type);
    mTimeToFinish = get<float>(Schematic::TimeToComplete).value_or(0.0f);

    mUnlocks = getArray<Object *>(Schematic::Unlocks);
    mSubCategories = getArray<Object *>(Schematic::SubCategories);
    mSmallSchematicIcon = get<Object *>(Schematic::SmallSchematicIcon);
    mSchematicCategory = get<Object *>(Schematic::SchematicCategory);

    mParentSchematic = get<Object *>(Schematic::DependsOnSchematic);
    mDependencies = getArray<Object *>(Schematic::SchematicDependencies);
    mHidden = get<bool>(Schematic::HiddenUntilDepsMet).value_or(false);

    mCost = getArray<ItemAmount>(Schematic::Cost, &ItemAmount::get);
    auto icon = get<PropertyHolder>(Schematic::SchematicIcon);

  }
} // namespace SAP
