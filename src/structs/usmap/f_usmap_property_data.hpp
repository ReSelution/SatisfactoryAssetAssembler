//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace SAP {

  enum class EUsmapPropertyType : uint8_t {
    BYTE_PROPERTY = 0,
    BOOL_PROPERTY = 1,
    INT_PROPERTY = 2,
    FLOAT_PROPERTY = 3,
    OBJECT_PROPERTY = 4,
    NAME_PROPERTY = 5,
    DELEGATE_PROPERTY = 6,
    DOUBLE_PROPERTY = 7,
    ARRAY_PROPERTY = 8,
    STRUCT_PROPERTY = 9,
    STR_PROPERTY = 10,
    TEXT_PROPERTY = 11,
    INTERFACE_PROPERTY = 12,
    MULTICAST_DELEGATE_PROPERTY = 13,
    WEAK_OBJECT_PROPERTY = 14,
    LAZY_OBJECT_PROPERTY = 15,
    ASSET_OBJECT_PROPERTY = 16,
    SOFT_OBJECT_PROPERTY = 17,
    U_INT64_PROPERTY = 18,
    U_INT32_PROPERTY = 19,
    U_INT16_PROPERTY = 20,
    INT64_PROPERTY = 21,
    INT16_PROPERTY = 22,
    INT8_PROPERTY = 23,
    MAP_PROPERTY = 24,
    SET_PROPERTY = 25,
    ENUM_PROPERTY = 26,
    FIELD_PATH_PROPERTY = 27,
    OPTIONAL_PROPERTY = 28,
    UNKNOWN = 255
  };

#pragma pack(push,1)
  struct FUsmapPropertyData {
    std::string Name;
    uint16_t SchemaIndex; // Index in der internen Liste beim Parsen
    uint8_t ArraySize;    // Für statische Arrays
    EUsmapPropertyType Type;

    // Zusätzliche Daten je nach Typ (StructName, InnerType für Arrays, etc.)
    std::string StructName;
    std::string EnumName;
    std::shared_ptr<FUsmapPropertyData> InnerType;
    std::shared_ptr<FUsmapPropertyData> ValueType; // Nur für MapProperty
  };

#pragma pack(pop)
}
