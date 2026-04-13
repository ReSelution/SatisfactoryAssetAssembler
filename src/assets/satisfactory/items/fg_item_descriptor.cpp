//
// Created by oleub on 30.01.26.
//

#include "fg_item_descriptor.hpp"

#include "fg/fg_exporter.hpp"

namespace SAA {
  void FGItemDescriptor::parse() {
    Object::parse();

    DBG_MARK_HANDLED(m_className, Item::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Item::MenuPriority);
    DBG_MARK_HANDLED(m_className, Item::RememberPickUp);
    DBG_MARK_HANDLED(m_className, Item::NeedsPickUpMarker);
    DBG_MARK_HANDLED(m_className, Item::CustomScannableDetails);
    DBG_MARK_HANDLED(m_className, Item::ScannerLightColor);
    DBG_MARK_HANDLED(m_className, Item::ShouldOverrideScannerDisplayText);
    mDisplayName = get<FText>(Base::DisplayName);
    mDescription = get<FText>(Base::Description);

    mAbbreviatedDisplayName = get<FText>(Item::AbbreviatedDisplayName);

    mCategory = get<Object *>(Item::Category);
    mPersistentBigIcon = get<Object *>(Item::PersistentBigIcon);
    mSmallIcon = get<Object *>(Item::SmallIcon);
    mConveyorMesh = get<Object *>(Item::ConveyorMesh);

    // Physikalische & Logistische Daten
    mStackSize = get<FName>(Item::StackSize); // Enum EStackSize
    mForm = get<FName>(Item::Form); // Enum EResourceForm (Solid, Liquid, Gas)
    mFluidColor = get<FColor>(Item::FluidColor); // StructProperty
    mGasColor = get<FColor>(Item::GasColor); // StructProperty
    mGasType = get<FName>(Item::GasType);

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

  void FGItemDescriptor::exprt() {
    FGExporter::encodeItem(this);
  }
} 
