//
// Created by oleub on 30.01.26.
//

#include "fg_item_descriptor_biomass.hpp"

#include "fg/fg_exporter.hpp"

namespace SAA {
  void FGItemDescriptorBiomass::parse() {
    Object::parse();
    DBG_MARK_HANDLED(m_className, Item::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Item::RememberPickUp);

    mDisplayName = get<SC::MagpieString>(Base::DisplayName);
    mDescription = get<SC::MagpieString>(Base::Description);
    mAbbreviatedDisplayName = get<SC::MagpieString>(Item::AbbreviatedDisplayName);

    mEnergyValue = get<float>(Item::EnergyValue).value_or(0.0f);
    mCategory = get<Object *>(Item::Category);
    mPersistentBigIcon = get<Object *>(Item::PersistentBigIcon);
    mSmallIcon = get<Object *>(Item::SmallIcon);
    mConveyorMesh = get<Object *>(Item::ConveyorMesh);

    mForm = get<FName>(Item::Form);
    mFluidColor = get<FColor>(Item::FluidColor); // StructProperty
    mStackSize = get<FName>(Item::StackSize); // Enum EStackSize
  }

  void FGItemDescriptorBiomass::exprt() {
    FGExporter::encodeItem(this);
  }
} 
