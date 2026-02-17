//
// Created by oleub on 09.02.26.
//

#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "byte_cursor.hpp"

namespace SAP {
  struct FPropertyValue;
  struct FPropertyTag;
  struct FPropertyTagData;
  enum class EPropertyTagType : uint32_t;

  enum class ReadType : uint8_t {
    NORMAL,
    MAP,
    ARRAY,
  };

  class PropertyHolder {

  public:
    PropertyHolder(std::string_view className = "PropertyHolder") : m_className(className) {};
    PropertyHolder(std::pmr::memory_resource *resource) : m_properties(resource) {};
    std::pmr::unordered_map<uint32_t, FPropertyTag> m_properties;
    void parseTaggedProperties(ByteCursor &cursor, ReadType readType = ReadType::NORMAL);
    static void releaseResources();
    PropertyHolder parseFallbackStruct(ByteCursor &cursor);

    template<typename T>
    std::optional<T> getProperty(std::string_view name) const;

    template<typename T>
    const std::pmr::vector<FPropertyValue> *getArray(std::string_view name) const;

  protected:
    AssetFileParser *m_owner = nullptr;
    std::string_view m_className;

  private:
    FPropertyTag parsePropertyTag(ByteCursor &cursor);

    FPropertyValue parseMapPropterty(ByteCursor &cursor, FPropertyTagData *tagData, uint32_t size);

    FPropertyTagData *parsePropertyTagData(ByteCursor &cursor, EPropertyTagType type,
                                           ReadType readType = ReadType::NORMAL);

    FPropertyValue parsePropertyValue(ByteCursor &cursor, FPropertyTag &tag, ReadType readType, uint32_t size);

    FPropertyValue parseStructPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                            uint32_t size);

    FPropertyValue parseTextPropertyValue(ByteCursor &cursor);

    FPropertyValue parseArrayPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type, uint32_t size);

    FPropertyValue parseObjectPropertyValue(ByteCursor &cursor);

    FPropertyValue parseSetPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type);

    FPropertyValue parseBoolPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type);
  };
} // namespace SAP
