//
// Created by oleub on 14.12.25.
//

#include "asset_object.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

#include "uasset_parser.hpp"
#include "usmap/f_usmap_mapping_data.hpp"


namespace SAA {
  void Object::parse() {
    parseProperties(m_cursor);
    DBG_TRACK_MAP(m_className, m_properties);
    // The guid flag follows the properties of every object except a class default object (UObject::Serialize).
    if (!isClassDefaultObject() && m_cursor.readBool() && m_cursor.remaining() >= 16) {
      m_cursor.read<FGuid>();
    }
  }

  void Object::exprt() {}

  Object::Object(Object *other) : classHash(other->classHash), m_className(other->m_className) {
    other->safeParse();

    m_objName      = other->m_objName;
    m_owner        = other->m_owner;
    m_cursor       = other->m_cursor;
    m_properties   = other->m_properties;
    other->m_inner = this;
    m_super        = other;
  }


  void Object::publishState(ObjectState state) {
    {
      std::lock_guard lock(m_mutex);
      m_state.store(state, std::memory_order_release);
    }
    m_cv.notify_all();
  }

  void Object::throwIfFailed() const {
    if (m_state.load(std::memory_order_acquire) == ObjectState::FAILED) [[unlikely]] {
      throw std::runtime_error("Parsing or exporting failed earlier: " + getFullName());
    }
  }

  void Object::safeParse() {
    if (m_state.load(std::memory_order_acquire) >= ObjectState::PARSED) {
      throwIfFailed();
      return;
    }
    ObjectState expected = ObjectState::NONE;
    if (m_state.compare_exchange_strong(expected, ObjectState::PARSING, std::memory_order_acq_rel)) {
      try {
        m_cursor = m_owner->getAsset(fileOffset, fileSize);
        parse();
      } catch (...) {
        // The threads waiting below have to be released as well.
        publishState(ObjectState::FAILED);
        throw;
      }
      publishState(ObjectState::PARSED);
      return;
    }

    {
      std::unique_lock lock(m_mutex);
      m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) >= ObjectState::PARSED; });
    }
    throwIfFailed();
  }

  sc::ecs::Entity Object::safeExport() {
    if (m_state.load(std::memory_order_acquire) == ObjectState::EXPORTED) {
      return entity;
    }
    safeParse();

    ObjectState expected = ObjectState::PARSED;
    if (m_state.compare_exchange_strong(expected, ObjectState::EXPORTING)) {
      try {
        exprt();
      } catch (...) {
        publishState(ObjectState::FAILED);
        throw;
      }
      publishState(ObjectState::EXPORTED);
      return entity;
    }

    {
      std::unique_lock lock(m_mutex);
      m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) >= ObjectState::EXPORTED; });
    }
    throwIfFailed();
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
