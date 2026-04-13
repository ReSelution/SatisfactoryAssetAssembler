//
// Created by oleub on 14.03.26.
//

#include "fg_descriptor_base.hpp"

namespace SAA {
  FGDescriptorBase::FGDescriptorBase(Object *object) : Object(object) {
  }

  void FGDescriptorBase::parse() {
    mDisplayName = get<FText>(Base::DisplayName);
    mDescription = get<FText>(Base::Description);

    mCategory = get<Object *>(Base::Category);
    mPersistentBigIcon = get<Object *>(Base::PersistentBigIcon);
    mSmallIcon = get<Object *>(Base::SmallIcon);
    mConveyorMesh = get<Object *>(Base::ConveyorMesh);

    mStackSize = get<FName>(Base::StackSize); // Enum EStackSize

  }

  void FGDescriptorBase::addProducedIn(Recipe *recipe) {
    SC::ChaosSpinLockGuard guard(mProducedInLock);
    if (!m_producedIn) {
      m_producedIn = m_owner->storage()->make<std::pmr::vector<Recipe*>>(m_owner->storage()->resource());
      m_producedIn->reserve(5);
    }
    m_producedIn->emplace_back(recipe);
  }

  void FGDescriptorBase::addUsedIn(Recipe *recipe) {
    SC::ChaosSpinLockGuard guard(mUsedInLock);
    if (!m_usedIn) {
      m_usedIn = m_owner->storage()->make<std::pmr::vector<Recipe*>>(m_owner->storage()->resource());
      m_usedIn->reserve(5);
    }
    m_usedIn->emplace_back(recipe);
  }
} // SAA
