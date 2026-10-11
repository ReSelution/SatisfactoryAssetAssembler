//
// Created by oleub on 30.01.26.
//

#include "fg_resource_descriptor.hpp"

#include "fg/fg_exporter.hpp"

namespace SAA {
  void FGResourceDescriptor::parse() {
    Object::parse();
    // Irrelevantes entsorgen
    DBG_MARK_HANDLED(m_className, Item::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Item::RememberPickUp);
    DBG_MARK_HANDLED(m_className, Item::ManualMiningAudioName);
    DBG_MARK_HANDLED(m_className, Item::ManualMiningParticle);
    DBG_MARK_HANDLED(m_className, Item::FactoryMiningParticle);
    DBG_MARK_HANDLED(m_className, Item::DestroyedParticle);
    DBG_MARK_HANDLED(m_className, Item::DepositMaterial);
    DBG_MARK_HANDLED(m_className, Item::DepositMesh);
    DBG_MARK_HANDLED(m_className, Item::DecalMaterial);

    // Relevante Daten für den Solver
    mDisplayName = get<sc::MagpieString>(Base::DisplayName);
    mDescription = get<sc::MagpieString>(Base::Description);
    mAbbreviatedDisplayName = get<sc::MagpieString>(Item::AbbreviatedDisplayName);

    mCategory = get<Object *>(Item::Category);
    mForm = get<FName>(Item::Form);
    mEnergyValue = get<float>(Item::EnergyValue).value_or(0.0f);
    mRadioactiveDecay = get<float>(Item::RadioactiveDecay).value_or(0.0f);

    // Icons & Visuals (für das UI des Calculators)
    mPersistentBigIcon = get<Object *>(Item::PersistentBigIcon);
    mSmallIcon = get<Object *>(Item::SmallIcon);
    mCompassTexture = get<Object *>(Item::CompassTexture);
    mPingColor = get<FLinearColor>(Item::PingColor);
    mConveyorMesh = get<Object *>(Item::ConveyorMesh);
    mIsAlienItem = get<bool>(Item::IsAlienItem).value_or(false);
    mStackSize = get<FName>(Item::StackSize);
    mFluidColor = get<FColor>(Item::FluidColor);
    mDepositMesh = get<Object *>(Item::DepositMesh);
  }

  void FGResourceDescriptor::exprt() {
    FGExporter::encodeItem(this);
  }
} 
