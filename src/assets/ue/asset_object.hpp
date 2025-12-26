//
// Created by oleub on 12.12.25.
//

#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "AssetFileParser.hpp"
#include "core/name.hpp"
#include "core/properties/f_property_tag.hpp"

namespace SAP {


  class Object {
    enum class ReadType : uint8_t {
      ZERO,
      NORMAL,
      MAP,
      ARRAY,
      OPTIONAL,
      RAW,
    };

  public:
    Object(std::string className = "Object") : m_className(std::move(className)) {};
    virtual ~Object() = default;

    [[nodiscard]] std::string getClassName() const { return m_className; }
    virtual void parse(size_t &cursor, AssetFileParser *owner);

    void setObjName(std::string name);

  protected:
    AssetFileParser *m_owner = nullptr;
    //std::vector<FPropertyTag> m_properties;
    std::unordered_map<uint32_t, FPropertyTag> m_properties;
    FName readFName(const std::vector<uint8_t> &asset, size_t &cursor);
    std::string getFullName() const;
    std::unique_ptr<Object> m_outer= nullptr;

    template <typename T, typename DefaultFn>
    T getOrDefault(uint32_t prop, DefaultFn&& makeDefault) const
    {
      auto it = m_properties.find(prop);
      if (it != m_properties.end())
      {
        return std::get<T>(it->second.Tag->Data);
      }

      return std::forward<DefaultFn>(makeDefault)();
    }

    template <typename E>
using enum_underlying_t = std::underlying_type_t<E>;

    template <typename E, typename DefaultFn>
    E getOrDefaultEnum(uint32_t prop, DefaultFn&& makeDefault) const
    {
      static_assert(std::is_enum_v<E>, "E must be an enum");

      using U = enum_underlying_t<E>;

      auto it = m_properties.find(prop);
      if (it != m_properties.end())
      {
        return static_cast<E>(
            std::get<U>(it->second.Tag->Data)
        );
      }

      return std::forward<DefaultFn>(makeDefault)();
    }
  private:
    void parseTaggedProperties(const std::vector<uint8_t> &asset, size_t &cursor);
    std::unique_ptr<FPropertyTagData> parsePropertyTagData
    (const std::vector<uint8_t> &asset, size_t &cursor, EPropertyTagType type);
    std::unique_ptr<FPropertyValue>
    parsePropertyValue
    (const std::vector<uint8_t> &asset,
     size_t &cursor,
     EPropertyTagType type,
     const FPropertyTagData *tagData,
     ReadType readType,
     int32_t size);

    std::unique_ptr<FPropertyValue> parseStructPropertyValue
    (const std::vector<uint8_t> &asset, size_t &cursor, std::string structType, ReadType type);
    std::unique_ptr<FPropertyValue> parseBytePropertyValue
    (const std::vector<uint8_t> &asset, size_t &cursor, ReadType type);

    const std::string m_className;
    std::string m_objName;
    FGuid m_objGuid;


  };


}
