//
// Created by oleub on 30.01.26.
//

#include "fg_resource_descriptor.hpp"

namespace SAA {
  void FGResourceDescriptor::parse() {
    Object::parse();
    // Irrelevantes entsorgen
    DBG_MARK_HANDLED(m_className, Resource::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Resource::RememberPickUp);
    DBG_MARK_HANDLED(m_className, Resource::ManualMiningAudioName);
    DBG_MARK_HANDLED(m_className, Resource::ManualMiningParticle);
    DBG_MARK_HANDLED(m_className, Resource::FactoryMiningParticle);
    DBG_MARK_HANDLED(m_className, Resource::DestroyedParticle);
    DBG_MARK_HANDLED(m_className, Resource::DepositMaterial);
    DBG_MARK_HANDLED(m_className, Resource::DepositMesh);
    DBG_MARK_HANDLED(m_className, Resource::DecalMaterial);

    // Relevante Daten für den Solver
    mDisplayName = get<FText>(Resource::DisplayName);
    mDescription = get<FText>(Resource::Description);
    mAbbreviatedDisplayName = get<FText>(Resource::AbbreviatedDisplayName);

    mCategory = get<Object *>(Resource::Category);
    mForm = get<FName>(Resource::Form);
    mEnergyValue = get<float>(Resource::EnergyValue).value_or(0.0f);
    mRadioactiveDecay = get<float>(Resource::RadioactiveDecay).value_or(0.0f);

    // Icons & Visuals (für das UI des Calculators)
    mPersistentBigIcon = get<Object *>(Resource::PersistentBigIcon);
    mSmallIcon = get<Object *>(Resource::SmallIcon);
    mCompassTexture = get<Object *>(Resource::CompassTexture);
    mPingColor = get<FLinearColor>(Resource::PingColor);
    mConveyorMesh = get<Object *>(Resource::ConveyorMesh);
    mIsAlienItem = get<bool>(Resource::IsAlienItem).value_or(false);
    mStackSize = get<FName>(Resource::StackSize);
    mFluidColor = get<FColor>(Resource::FluidColor);
    mDepositMesh = get<Object *>(Resource::DepositMesh);
  }
} // namespace SAP
