//
// Created by oleub on 08.03.26.
//

#include "fg_factory.hpp"

#include "building/fg_building_base.hpp"
#include "items/fg_item_base.hpp"

namespace SAA {
  FGItemBase *FGFactory::createItem(Object *obj) {
    if (!obj) { return nullptr; }
    if (obj->getClassHash() != Object::StaticHASH) { return static_cast<FGItemBase *>(obj);; }
    if (obj->inner()) { return static_cast<FGItemBase *>(obj->inner()); }

    std::lock_guard lock(obj->m_factoryLock);

    if (obj->inner()) { return static_cast<FGItemBase *>(obj->inner()); }
    return obj->storage()->make<FGItemBase>(obj);
  }

  FGBuildingBase *FGFactory::createBuilding(Object *obj) {
    if (!obj) { return nullptr; }
    if (obj->getClassHash() !=  Object::StaticHASH) { return static_cast<FGBuildingBase *>(obj); }
    if (obj->inner()) { return static_cast<FGBuildingBase *>(obj->inner()); }

    std::lock_guard lock(obj->m_factoryLock);

    if (obj->inner()) { return static_cast<FGBuildingBase *>(obj->inner()); }
    return obj->storage()->make<FGBuildingBase>(obj);
  }

  FGDescriptorBase *FGFactory::createBase(Object *obj) {
    if (!obj) { return nullptr; }
    if (obj->getClassHash() !=  Object::StaticHASH) { return static_cast<FGDescriptorBase *>(obj); }
    if (obj->inner()) return static_cast<FGDescriptorBase *>(obj->inner());
    std::lock_guard lock(obj->m_factoryLock);

    if (obj->inner()) { return static_cast<FGDescriptorBase *>(obj->inner()); }
    return obj->storage()->make<FGDescriptorBase>(obj);
  }
} // SAA
