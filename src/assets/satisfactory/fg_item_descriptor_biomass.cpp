//
// Created by oleub on 30.01.26.
//

#include "fg_item_descriptor_biomass.hpp"

namespace SAA {
  void FGItemDescriptorBiomass::parse() {
    Object::parse();
    DBG_MARK_HANDLED(m_className, Biomass::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Biomass::RememberPickUp);

    mDisplayName       = get<FText>(Biomass::DisplayName);
    mDescription       = get<FText>(Biomass::Description);
    mAbbreviatedDisplayName   = get<FText>(Biomass::AbbreviatedDisplayName);

    mEnergyValue = get<float>(Biomass::EnergyValue).value_or(0.0f);
    mCategory          = get<Object*>(Biomass::Category);
    mPersistentBigIcon = get<Object*>(Biomass::PersistentBigIcon);
    mSmallIcon         = get<Object*>(Biomass::SmallIcon);
    mConveyorMesh      = get<Object*>(Biomass::ConveyorMesh);

    mForm = get<FName>(Biomass::Form);
    mFluidColor        = get<FColor>(Biomass::FluidColor); // StructProperty
    mStackSize         = get<FName>(Biomass::StackSize); // Enum EStackSize


  }
} // namespace SAP
