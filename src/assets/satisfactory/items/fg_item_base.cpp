//
// Created by oleub on 01.03.26.
//

#include "fg_item_base.hpp"

namespace SAA {
  FGItemBase::FGItemBase(Object *object) : FGDescriptorBase(object) {
  }

  void FGItemBase::parse() {
    FGDescriptorBase::parse();
    DBG_MARK_HANDLED(m_className, Item::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Item::MenuPriority);
    DBG_MARK_HANDLED(m_className, Item::RememberPickUp);
    DBG_MARK_HANDLED(m_className, Item::NeedsPickUpMarker);
    DBG_MARK_HANDLED(m_className, Item::CachedStackSize);


    mAbbreviatedDisplayName = get<FText>(Item::AbbreviatedDisplayName);


    // Physikalische & Logistische Daten
    mForm = get<FName>(Item::Form); // Enum EResourceForm (Solid, Liquid, Gas)
    mFluidColor = get<FColor>(Item::FluidColor); // StructProperty
    mGasColor = get<FColor>(Item::GasColor); // StructProperty

    // Gameplay & Simplex-Parameter
    mEnergyValue = get<float>(Item::EnergyValue).value_or(0.0f);
    mRadioactiveDecay = get<float>(Item::RadioactiveDecay).value_or(0.0f);
    mCanBeDiscarded = get<bool>(Item::CanBeDiscarded).value_or(true);
    mIsAlienItem = get<bool>(Item::IsAlienItem).value_or(false);
    mItemIndex = get<int32_t>(Item::ItemIndex).value_or(0);

    // Scanning-Daten (Wichtig für Harddrive/Resource-Search Logik)
    mScannableType = get<FName>(Item::ScannableType);
    mClassToScanFor = get<SoftObj>(Item::ClassToScanFor);
    mScannerDisplayText = get<FText>(Item::ScannerDisplayText);
    mRequiredSchematic = get<Object *>(Item::RequiredSchematicToScan);
  }
} // SAP
