//
// Created by oleub on 14.03.26.
//

#include "fg_descriptor_base.hpp"

#include <mutex>

namespace SAA {
  FGDescriptorBase::FGDescriptorBase(Object *object) : Object(object) {
  }

  void FGDescriptorBase::parse() {
    mDisplayName = get<sc::MagpieString>(Base::DisplayName);
    mDescription = get<sc::MagpieString>(Base::Description);

    mCategory = get<Object *>(Base::Category);
    mPersistentBigIcon = get<Object *>(Base::PersistentBigIcon);
    mSmallIcon = get<Object *>(Base::SmallIcon);
    mConveyorMesh = get<Object *>(Base::ConveyorMesh);

    mStackSize = get<FName>(Base::StackSize); // Enum EStackSize

  }

  void FGDescriptorBase::addProducedIn(Recipe *recipe) {
    std::scoped_lock guard(mProducedInLock);
    if (!m_producedIn) {
      m_producedIn = m_owner->storage()->makePmr<std::pmr::vector<Recipe *>>();
      m_producedIn->reserve(5);
    }
    m_producedIn->emplace_back(recipe);
  }

  void FGDescriptorBase::addUsedIn(Recipe *recipe) {
    std::scoped_lock guard(mUsedInLock);
    if (!m_usedIn) {
      m_usedIn = m_owner->storage()->makePmr<std::pmr::vector<Recipe *>>();
      m_usedIn->reserve(5);
    }
    m_usedIn->emplace_back(recipe);
  }
} // SAA
