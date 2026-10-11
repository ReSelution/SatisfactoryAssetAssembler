//
// Created by oleub on 24.12.25.
//

#include "exporter.hpp"

#include <cassert>

#include "asset_object.hpp"

namespace SAA {
  bool Exporter::checkPath(std::filesystem::path path) {
    std::filesystem::path dir(path.parent_path());
    if (!dir.empty() && !std::filesystem::exists(dir)) {
      std::filesystem::create_directories(dir);
    }
    return std::filesystem::exists(path);
  }

  // TODO: Nicht threadsicher. Rufen zwei Threads getEntity() fuer dasselbe Objekt gleichzeitig auf, legen beide eine
  // Entity an und eine geht verloren; obj->entity wird dabei ohne Synchronisation gelesen und geschrieben. Der
  // Lock unten ist auskommentiert (sc::SpinLockGuard waere der Nachfolger von SC::ChaosSpinLockGuard).
  sc::ecs::Entity Exporter::getEntity(Object *obj) {

    if (obj->entity)[[likely]] {
      return obj->entity;
    }
//    sc::SpinLockGuard lock(obj->entityCreationLock);
//    if (obj->entity) {
//      return obj->entity;
//    }
    assert(reg != nullptr);
    obj->entity = reg->create();
    return obj->entity;
  }

  void Exporter::setECS(sc::ecs::Registry *r) {
    reg = r;
  }
}
