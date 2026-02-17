//
// Created by oleub on 12.12.25.
//

#pragma once

#include "AssetFileParser.hpp"
#include "byte_cursor.hpp"
#include "core/f_guid.hpp"
#include "core/hash.hpp"
#include "logger.hpp"

#include "properties/property_holder.hpp"
#include "properties/f_property_tag.hpp"

namespace SAP {
  class AssetFileParser;

  class Object : public PropertyHolder {

    enum class ObjectState : uint8_t {
      NONE,
      PARSING,
      LOADED,
    };

  public:
    Object(std::string_view className = "Object"):PropertyHolder(className) {

    };
    virtual ~Object() = default;

    [[nodiscard]] std::string_view getClassName() const { return m_className; }
    virtual uint32_t getClassHash() { return HASH; }
    virtual void exportAsset();
    void safeParse();
    void setOwner(AssetFileParser *parser);
    void setObjName(std::string_view name);
    void setCursor(const ByteCursor &cursor);
    std::string getFullName() const;
    std::string_view objName() const { return m_objName; }
    std::string_view path() const { return m_owner->path(); }

  protected:
    virtual void parse();
    ByteCursor m_cursor;
    std::unique_ptr<Object> m_outer = nullptr;

  private:
    static constexpr uint32_t HASH = fnv1a("Object");


    std::string_view m_objName;
    FGuid m_objGuid{};

    std::atomic<ObjectState> m_state = ObjectState::NONE;
    std::mutex m_mutex;
    std::condition_variable m_cv;
  };
} // namespace SAP
