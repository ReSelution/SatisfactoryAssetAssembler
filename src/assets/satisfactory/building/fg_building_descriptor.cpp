//
// Created by oleub on 30.01.26.
//

#include "fg_building_descriptor.hpp"

#include "fg/fg_exporter.hpp"
#include "logger/chaos_logger.hpp"
LOG_ALIAS(errLog, "SAA", "FGBuildingDescriptor")

namespace SAA {
  void FGBuildingDescriptor::parse() {
    Object::parse();
    DBG_MARK_HANDLED(m_className, Building::UsesDistanceForZooping);
    DBG_MARK_HANDLED(m_className, Building::CachedStackSize);
    DBG_MARK_HANDLED(m_className, Building::UseDisplayNameAndDescription);
    DBG_MARK_HANDLED(m_className, Building::QuickSwitchGroup);
    DBG_MARK_HANDLED(m_className, Building::MenuPriority);

    mCategory = get<Object*>(Base::Category);
    mPersistentBigIcon = get<Object*>(Base::PersistentBigIcon);
    mBuildableClass = get<Object*>(Building::BuildableClass).value();
    mSubCategories = getArray<Object*>(Building::SubCategories);
    mConveyorMesh = get<Object*>(Base::ConveyorMesh);
    mStackSize = get<FName>(Base::StackSize);
    mSmallIcon = get<Object*>(Base::SmallIcon);
  }

  void FGBuildingDescriptor::exprt() {
    FGExporter::encodeBuilding(this);
  }
} 
