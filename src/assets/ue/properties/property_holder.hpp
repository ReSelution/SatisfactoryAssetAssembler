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
    PropertyHolder(std::string_view className = "PropertyHolder") : m_className(className) {
    };

    PropertyHolder(std::pmr::memory_resource *resource) : m_properties(resource) {
    };
    std::pmr::unordered_map<uint32_t, FPropertyTag> m_properties;

    void parseTaggedProperties(ByteCursor &cursor, ReadType readType = ReadType::NORMAL);

    static void releaseResources();

    static PropertyHolder parseFallbackStruct(ByteCursor &cursor);

    template<typename T>
    std::optional<T> get(uint32_t hash) const;

    template<typename T, typename Func>
    std::optional<T> get(uint32_t hash, Func &&processor) const;


    template<typename T>
    std::span<T> getArray(uint32_t hash) const;

    template<typename T, typename Func>
    std::span<T> getArray(uint32_t hash, Func &&processor) const;

  protected:
    AssetFileParser *m_owner = nullptr;
    std::string m_className;

  private:
    static FPropertyTag parsePropertyTag(ByteCursor &cursor);

    static FPropertyValue parseMapProperty(ByteCursor &cursor, FPropertyTagData *tagData, uint32_t size);

    static FPropertyTagData *parsePropertyTagData(ByteCursor &cursor, EPropertyTagType type);

    static FPropertyValue parsePropertyValue(ByteCursor &cursor, FPropertyTag &tag, ReadType readType, uint32_t size);

    static FPropertyValue parseStructPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                                   uint32_t size);

    static FPropertyValue parseTextPropertyValue(ByteCursor &cursor);

    static FPropertyValue parseArrayPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                                  uint32_t size);

    static FPropertyValue parseObjectPropertyValue(ByteCursor &cursor);

    static FPropertyValue parseSetPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData);

    static FPropertyValue parseBoolPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type);
  };
} // namespace SAP
