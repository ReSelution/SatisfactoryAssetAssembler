//
// Created by oleub on 30.01.26.
//

#include "fg_building_descriptor.hpp"

namespace SAA {
  void FGBuildingDescriptor::parse() {
    Object::parse();
    DBG_MARK_HANDLED(m_className, Building::UsesDistanceForZooping);
    DBG_MARK_HANDLED(m_className, Building::MenuPriority);
    DBG_MARK_HANDLED(m_className, Building::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Building::UseDisplayNameAndDescription);
    DBG_MARK_HANDLED(m_className, Building::QuickSwitchGroup);

    mCategory = get<Object*>(Building::Category);
    mPersistentBigIcon = get<Object*>(Building::PersistentBigIcon);
    mSmallIcon = get<Object*>(Building::SmallIcon);
    mBuildableClass = get<Object*>(Building::BuildableClass).value();
    mSubCategories = getArray<Object*>(Building::SubCategories);
    mConveyorMesh = get<Object*>(Building::ConveyorMesh);
    mStackSize = get<FName>(Building::StackSize);
  }
} // namespace SAP
