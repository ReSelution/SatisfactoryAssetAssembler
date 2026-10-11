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
      // TODO: Debug-Ueberbleibsel (Haltepunkt-Anker), gibt eine unused-variable-Warnung.
      int debig;
    }
    parseProperties(m_cursor);
    DBG_TRACK_MAP(m_className, m_properties);
    if (m_cursor.readBool() && m_cursor.remaining() >= 16) {
      m_cursor.read<FGuid>();
    }
  }

  void Object::exprt() {}

  Object::Object(Object *other) : PropertyHolder(other->m_className), classHash(other->classHash) {
    other->safeParse();

    m_objName      = other->m_objName;
    m_owner        = other->m_owner;
    m_cursor       = other->m_cursor;
    m_properties   = other->m_properties;
    other->m_inner = this;
    m_super        = other;
  }


  // TODO: Zwei Dinge, beide schon vor der Portierung so:
  // - Wirft parse(), bleibt m_state auf PARSING und alle Threads im m_cv.wait() unten haengen fuer immer.
  // - Der schnelle Pfad prueft nur == PARSED. Nach dem Export (EXPORTING/EXPORTED) nimmt jeder safeParse()-Aufruf
  //   den Umweg ueber den fehlschlagenden CAS und m_mutex. >= PARSED wuerde das vermeiden.
  void Object::safeParse() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::PARSED) {
      return;
    }
    ObjectState expected = ObjectState::NONE;
    if (m_state.compare_exchange_strong(expected, ObjectState::PARSING, std::memory_order_acq_rel)) {
      {
        m_cursor = m_owner->getAsset(fileOffset, fileSize);
        parse();
      }
      {
        std::lock_guard lock(m_mutex);
        m_state.store(ObjectState::PARSED, std::memory_order_release);
      }
      m_cv.notify_all();
      return;
    }

    std::unique_lock lock(m_mutex);
    m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) >= ObjectState::PARSED; });
  }

  sc::ecs::Entity Object::safeExport() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::EXPORTED) {
      return entity;
    }
    safeParse();

    ObjectState expected = ObjectState::PARSED;
    if (m_state.compare_exchange_strong(expected, ObjectState::EXPORTING)) {
      {
        exprt();
      }

      {
        std::lock_guard lock(m_mutex);
        m_state.store(ObjectState::EXPORTED, std::memory_order_release);
      }
      m_cv.notify_all();
      return entity;
    }

    std::unique_lock lock(m_mutex);
    m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) >= ObjectState::EXPORTED; });
    return entity;
  }

  void SAA::Object::setOwner(AssetFileParser *parser) { m_owner = parser; }

  std::string Object::getFullName() const {
    std::string name = std::string(m_className);
    name += " ";
    name += m_owner->path();
    name += ".";
    name += m_objName;
    return name;
  }

  void Object::setObjName(std::string_view name) { m_objName = name; }

} // namespace SAA
