//
// Created by oleub on 09.02.26.
//

#pragma once
#include <cstdint>
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

  protected:
    AssetFileParser *m_owner = nullptr;
   HeapStorage<> m_stringStorage;


  public:
    PropertyHolder() =default;
    std::unordered_map<uint32_t, FPropertyTag> m_properties;
    void parseTaggedProperties(ByteCursor &cursor, ReadType readType = ReadType::NORMAL);
    void releaseResources();
    PropertyHolder parseFallbackStruct(ByteCursor &cursor);

  private:



    FPropertyTag parsePropertyTag(ByteCursor &cursor);

    FPropertyValue parseMapPropterty(ByteCursor &cursor,  FPropertyTagData *tagData);

    FPropertyTagData *parsePropertyTagData
    (ByteCursor &cursor, EPropertyTagType type, ReadType readType = ReadType::NORMAL);

    FPropertyValue parsePropertyValue(ByteCursor &cursor, FPropertyTag &tag, ReadType readType, uint32_t size);

    FPropertyValue parseStructPropertyValue
    (ByteCursor &cursor,  FPropertyTagData *tagData, ReadType type, uint32_t size);

    FPropertyValue parseTextPropertyValue(ByteCursor &cursor);


    FPropertyValue parseArrayPropertyValue(ByteCursor &cursor,  FPropertyTagData *tagData, ReadType type);

    FPropertyValue parseObjectPropertyValue(ByteCursor &cursor);

    FPropertyValue parseSetPropertyValue(ByteCursor &cursor,  FPropertyTagData *tagData, ReadType type);

    FPropertyValue parseBoolPropertyValue(ByteCursor &cursor,  FPropertyTagData *tagData, ReadType type);

  };
} // SAP
