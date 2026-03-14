//
// Created by oleub on 12.12.25.
//

#pragma once

#include "AssetFileParser.hpp"
#include "byte_cursor.hpp"
#include "core/f_guid.hpp"
#include "core/hash.hpp"

#include "properties/property_holder.hpp"
#include "properties/f_property_tag.hpp"

namespace SAA {
  class AssetFileParser;

  class Object : public PropertyHolder {
    enum class ObjectState : uint8_t {
      NONE,
      PARSING,
      PARSED,
      EXPORTING,
      EXPORTED // Finaler Status
    };

  public:
    explicit Object(std::string_view className = "Object") : PropertyHolder(className) {};
    explicit Object(Object* other);

    virtual ~Object() = default;


    [[nodiscard]] std::string_view getClassName() const { return m_className; }
    virtual uint32_t getClassHash() { return HASH; }


    void safeParse();

    void safeExport();

    void setOwner(AssetFileParser *parser);

    void setObjName(std::string_view name);

    void setCursor(const ByteCursor &cursor);

    std::string getFullName() const;

    std::string_view objName() const { return m_objName; }
    std::string_view path() const { return m_owner->path(); }

    [[nodiscard]] Object *outer() const {
      return m_outer;
    }
    void outer(Object *outer) {
      m_outer = outer;
    }

    [[nodiscard]] Object *super() const {
      return m_super;
    }

    void super(Object *super) {
      m_super = super;
    }

    SC::ChaosSlab * storage() const {
      return m_owner->storage();
    }

 [[nodiscard]] Object * inner() const {
  return m_inner;
    }

    void setInner(Object *inner) {
      m_inner = inner;
    }

  protected:
    virtual void parse();

    virtual void exprt();

    ByteCursor m_cursor{};
    Object *m_outer = nullptr;
    Object *m_super = nullptr;
    Object *m_inner = nullptr;

  private:
    static constexpr uint32_t HASH = fnv1a("Object");


    std::string_view m_objName;
    FGuid m_objGuid{};

    std::atomic<ObjectState> m_state = ObjectState::NONE;
    std::mutex m_mutex;
    std::condition_variable m_cv;
  };
} // namespace SAP
