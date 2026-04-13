//
// Created by oleub on 14.12.25.
//

#include "asset_object.hpp"

#include <cassert>

#include "uasset_parser.hpp"
#include "usmap/f_usmap_mapping_data.hpp"


namespace SAA {
  void Object::parse() {
    if (getClassName() == "FGBuildableResourceSink" && objName() == "Build_ResourceSink_C") {
      int debig;
    }
    parseProperties(m_cursor);
    DBG_TRACK_MAP(m_className, m_properties);
    if (m_cursor.readBool() && m_cursor.remaining() >= 16) {
      m_cursor.read<FGuid>();
    }
  }

  void Object::exprt() {
  }

  Object::Object(Object *other) : classHash(other->classHash),PropertyHolder(other->m_className) {
    other->safeParse();

    m_objName = other->m_objName;
    m_owner = other->m_owner;
    m_cursor = other->m_cursor;
    m_properties = other->m_properties;
    other->m_inner = this;
    m_super = other;
  }


  void Object::safeParse() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::PARSED) {
      return;
    }
    ObjectState expected = ObjectState::NONE;
    if (m_state.compare_exchange_strong(expected, ObjectState::PARSING, std::memory_order_acq_rel)) {
      {
        ZoneScopedN("Object::parsing")
        parse();
      }
      {
        std::lock_guard lock(m_mutex);
        m_state.store(ObjectState::PARSED, std::memory_order_release);
      }
      m_cv.notify_all();
      return;
    }

    ZoneScopedN("Object::awaiting Parsing")
    std::unique_lock lock(m_mutex);
    m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) >= ObjectState::PARSED; });
  }

  SC::ChaosEntity Object::safeExport() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::EXPORTED) {
      return entity;
    }
    safeParse();

    ObjectState expected = ObjectState::PARSED;
    if (m_state.compare_exchange_strong(expected, ObjectState::EXPORTING)) {
      {
        ZoneScopedN("Object::Exporting")
        exprt();
      }

      {
        std::lock_guard lock(m_mutex);
        m_state.store(ObjectState::EXPORTED, std::memory_order_release);
      }
      m_cv.notify_all();
      return entity;
    }

    ZoneScopedN("Object::awaiting Export")
    std::unique_lock lock(m_mutex);
    m_cv.wait(lock, [this] {
      return m_state.load(std::memory_order_acquire) >= ObjectState::EXPORTED;
    });
    return entity;
  }

void SAA::Object::setOwner(AssetFileParser *parser) {
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

}
