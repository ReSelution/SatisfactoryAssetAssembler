//
// Created by oleub on 24.12.25.
//

#include "exporter.hpp"

#include "asset_object.hpp"

namespace SAA {
  bool Exporter::checkPath(std::filesystem::path path) {
    std::filesystem::path dir(path.parent_path());
    if (!dir.empty() && !std::filesystem::exists(dir)) {
      std::filesystem::create_directories(dir);
    }
    return std::filesystem::exists(path);
  }

  SC::ChaosEntity Exporter::getEntity(Object *obj) {

    if (obj->entity)[[likely]] {
      return obj->entity;
    }
//    SC::ChaosSpinLockGuard lock(obj->entityCreationLock);
//    if (obj->entity) {
//      return obj->entity;
//    }
    assert(reg != nullptr);
    obj->entity = reg->create();
    return obj->entity;
  }

  void Exporter::setECS(SC::ChaosRegistry *r) {
    reg = r;
  }
}
