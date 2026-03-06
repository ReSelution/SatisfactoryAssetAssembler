//
// Created by oleub on 14.12.25.
//

#include "ue/asset_object.hpp"

#include "uasset_parser.hpp"
#include "usmap/f_usmap_mapping_data.hpp"


namespace SAA {
  void Object::parse() {
    parseProperties(m_cursor);
    DBG_TRACK_MAP(m_className,m_properties);
    if (m_cursor.readBool() && m_cursor.remaining() >=16 ) {
      m_cursor.read<FGuid>();
    }
  }

  void Object::exprt() {
  }

  void Object::exportAsset() {}

  void Object::safeParse() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::PARSED) {
      return;
    }
    ObjectState expected = ObjectState::NONE;
    if (m_state.compare_exchange_strong(expected, ObjectState::PARSING)) {
      parse();
      releaseResources();
      m_state = ObjectState::PARSED;
      m_cv.notify_all();
      return;
    }

    if (expected == ObjectState::PARSING || m_state == ObjectState::PARSING) {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this] { return m_state != ObjectState::PARSING; });
    }
  }

  void Object::safeExport() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::EXPORTED) {
      return;
    }
    safeParse();

    ObjectState expected = ObjectState::PARSED;
    if (m_state.compare_exchange_strong(expected, ObjectState::EXPORTING)) {

      exprt();

      m_state.store(ObjectState::EXPORTED, std::memory_order_release);
      m_cv.notify_all();
      return;
    }
    if (m_state.load(std::memory_order_acquire) == ObjectState::EXPORTING) {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this] {
          return m_state.load(std::memory_order_acquire) == ObjectState::EXPORTED;
      });
    }
  }

  void Object::setOwner(AssetFileParser *parser) {
    m_owner = parser;
  }

  std::string Object::getFullName() const {
    std::string name = std::string(m_className);
    name += " ";
    name += m_owner->path();
    name += ".";
    name += m_objName;
    return name;
  }

  void Object::setObjName(std::string_view name) { m_objName = name; }


  void Object::setCursor(const ByteCursor &cursor) {
    assert(m_state.load(std::memory_order_acquire) == ObjectState::NONE);
    m_cursor = cursor;
  }

} // namespace SAP


