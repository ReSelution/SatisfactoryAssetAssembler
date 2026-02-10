//
// Created by oleub on 14.12.25.
//

#include "ue/asset_object.hpp"

#include "uasset_parser.hpp"
#include "logger.hpp"

#include "core/f_color.hpp"
#include "core/f_text.hpp"
#include "core/per_quality_level_props.hpp"

#include "usmap/f_usmap_mapping_data.hpp"

#include "core/math/f_vector.hpp"

namespace SAP {
  void Object::parse() {
    parseTaggedProperties(m_cursor);
    bool hasGuid = m_cursor.readBool();
    if (hasGuid) { m_cursor.read<FGuid>(); }
  }

  void Object::exportAsset() {}

  void Object::safeParse() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::LOADED) { return; }
    ObjectState expected = ObjectState::NONE;
    if (m_state.compare_exchange_strong(expected, ObjectState::PARSING)) {
      parse();
      releaseResources();
      m_state = ObjectState::LOADED;
      m_cv.notify_all();
      return;
    }

    if (expected == ObjectState::PARSING || m_state == ObjectState::PARSING) {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this] { return m_state != ObjectState::PARSING; });
    }
  }

  std::string Object::getFullName() const {
    std::string name = std::string(m_className);
    name += " " ;
    name += m_owner->path();
    name += ".";
    name += m_objName;
    return name;
  }

  void Object::setObjName(std::string_view name) { m_objName = name; }

  void Object::setCursor(const ByteCursor &cursor) { m_cursor = cursor; }

}
