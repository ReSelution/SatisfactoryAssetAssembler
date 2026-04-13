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
    if (obj->entity) {
      return obj->entity;
    }
    SC::ChaosSpinLockGuard lock(obj->entityCreationLock);
    if (obj->entity) {
      return obj->entity;
    }
    obj->entity = reg.create();
    return obj->entity;
  }
} 
