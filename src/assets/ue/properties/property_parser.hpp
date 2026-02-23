#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

#include "byte_cursor.hpp"
#include "version.hpp"
#include "f_property_tag.hpp"

namespace SAP {
  class PropertyHolder;

#define PARSER_NOT_IMPLEMENTED(func) \
    throw std::runtime_error(std::string(func) + " for Version " + std::to_string(static_cast<uint32_t>(Version)) + " not implemented yet!")

  template<EUE5Version Version>
  class PropertyParser {
  public:
    static void parse(PropertyHolder *holder, ByteCursor &cursor) { PARSER_NOT_IMPLEMENTED("parse"); }
    static PropertyHolder parseFallbackStruct(ByteCursor &cursor) { PARSER_NOT_IMPLEMENTED("parseFallbackStruct"); }
    static FPropertyTag parsePropertyTag(ByteCursor &cursor) { PARSER_NOT_IMPLEMENTED("parsePropertyTag"); }

    static FPropertyTagData *parsePropertyTagData(ByteCursor &cursor, EPropertyTagType type) {
      PARSER_NOT_IMPLEMENTED("parsePropertyTagData");
    }

    static FPropertyTagData *parsePropertyTagData(EPropertyTagType type, std::span<FPropertyTypeNameNode> typeName,
                                                  const FName &name) {
      PARSER_NOT_IMPLEMENTED("parsePropertyTagData");
    }

    static FPropertyValue parsePropertyValue(ByteCursor &cursor, FPropertyTag &tag, ReadType readType, uint32_t size) {
      PARSER_NOT_IMPLEMENTED("parsePropertyValue");
    }

    static FPropertyValue parseMapProperty(ByteCursor &cursor, FPropertyTagData *tagData, uint32_t size) {
      PARSER_NOT_IMPLEMENTED("parseMapProperty");
    }

    static FPropertyValue parseStructPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                                   uint32_t size) {
      PARSER_NOT_IMPLEMENTED("parseStructPropertyValue");
    }

    static FPropertyValue parseArrayPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                                  uint32_t size) { PARSER_NOT_IMPLEMENTED("parseArrayPropertyValue"); }

    static FPropertyValue parseSetPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData) {
      PARSER_NOT_IMPLEMENTED("parseSetPropertyValue");
    }

    static FPropertyValue parseBoolPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type) {
      PARSER_NOT_IMPLEMENTED("parseBoolPropertyValue");
    }

    static FPropertyValue parseTextPropertyValue(ByteCursor &cursor) {
      PARSER_NOT_IMPLEMENTED("parseTextPropertyValue");
    }

    static FPropertyValue parseObjectPropertyValue(ByteCursor &cursor) {
      PARSER_NOT_IMPLEMENTED("parseObjectPropertyValue");
    }
  };

#undef PARSER_NOT_IMPLEMENTED

  using PP3 = PropertyParser<EUE5Version::UE5_3>;

  template<>
  void PP3::parse(PropertyHolder *holder, ByteCursor &cursor);

  template<>
  PropertyHolder PP3::parseFallbackStruct(ByteCursor &cursor);

  template<>
  FPropertyTag PP3::parsePropertyTag(ByteCursor &cursor);

  template<>
  FPropertyTagData *PP3::parsePropertyTagData(ByteCursor &cursor, EPropertyTagType type);

  template<>
  FPropertyValue PP3::parsePropertyValue(ByteCursor &cursor, FPropertyTag &tag, ReadType readType, uint32_t size);

  template<>
  FPropertyValue PP3::parseMapProperty(ByteCursor &cursor, FPropertyTagData *tagData, uint32_t size);

  template<>
  FPropertyValue PP3::parseStructPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                               uint32_t size);

  template<>
  FPropertyValue PP3::parseArrayPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                              uint32_t size);

  template<>
  FPropertyValue PP3::parseSetPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData);

  template<>
  FPropertyValue PP3::parseBoolPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type);

  template<>
  FPropertyValue PP3::parseTextPropertyValue(ByteCursor &cursor);

  template<>
  FPropertyValue PP3::parseObjectPropertyValue(ByteCursor &cursor);


  using PP6 = PropertyParser<EUE5Version::UE5_6>;

  template<>
  void PP6::parse(PropertyHolder *holder, ByteCursor &cursor);

  template<>
  FPropertyTag PP6::parsePropertyTag(ByteCursor &cursor);

  template<>
  FPropertyTagData *PP6::parsePropertyTagData(EPropertyTagType type, std::span<FPropertyTypeNameNode> typeName,
                                              const FName &name);

  template<>
  FPropertyTag PP6::parsePropertyTag(ByteCursor &cursor);

  template<>
  FPropertyTagData *PP6::parsePropertyTagData(ByteCursor &cursor, EPropertyTagType type);

  template<>
  FPropertyValue PP6::parsePropertyValue(ByteCursor &cursor, FPropertyTag &tag, ReadType readType, uint32_t size);

  template<>
  FPropertyValue PP6::parseMapProperty(ByteCursor &cursor, FPropertyTagData *tagData, uint32_t size);

  template<>
  FPropertyValue PP6::parseArrayPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                              uint32_t size);

  template<>
  FPropertyValue PP6::parseStructPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                               uint32_t size);

  template<>
  FPropertyValue PP6::parseSetPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData);

  template<>
  FPropertyValue PP6::parseBoolPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type);

  template<>
  FPropertyValue PP6::parseTextPropertyValue(ByteCursor &cursor);

  template<>
  FPropertyValue PP6::parseObjectPropertyValue(ByteCursor &cursor);
} // namespace SAP
