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

  // Needs no lock of its own: it is only called from obj's own exprt(), which Object::safeExport() runs once per
  // object (state EXPORTING). Other threads read obj->entity through safeExport() after EXPORTED is published.
  // Registry::create() may be called from several threads.
  sc::ecs::Entity Exporter::getEntity(Object *obj) {

    if (obj->entity)[[likely]] {
      return obj->entity;
    }
    assert(reg != nullptr);
    obj->entity = reg->create();
    return obj->entity;
  }

  void Exporter::setECS(sc::ecs::Registry *r) {
    reg = r;
  }
}
