//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

#include "core/f_guid.hpp"
#include "core/hash.hpp"
#include "core/name.hpp"

namespace SAP {
  struct FPropertyValue;

  enum class ESatisfactoryStruct : uint32_t {
    Unknown          = 0,

    Vector           = fnv1a("Vector"),
    Vector2D         = fnv1a("Vector2D"),
    Rotator          = fnv1a("Rotator"),
    Transform        = fnv1a("Transform"),
    Box              = fnv1a("Box"),
    Guid             = fnv1a("Guid"),
    LinearColor      = fnv1a("LinearColor"),
    IntPoint         = fnv1a("IntPoint"),
  };



    enum class EPropertyTagType : uint32_t {
    Unknown           = 0,
    Array             = fnv1a("ArrayProperty"),
    AssetObject       = fnv1a("AssetObjectProperty"),
    AssetClass        = fnv1a("AssetClassProperty"),
    Bool              = fnv1a("BoolProperty"),
    Byte              = fnv1a("ByteProperty"),
    Class             = fnv1a("ClassProperty"),
    Delegate          = fnv1a("DelegateProperty"),
    Double            = fnv1a("DoubleProperty"),
    Enum              = fnv1a("EnumProperty"),
    FieldPath         = fnv1a("FieldPathProperty"),
    Float             = fnv1a("FloatProperty"),
    Int16             = fnv1a("Int16Property"),
    Int64             = fnv1a("Int64Property"),
    Int8              = fnv1a("Int8Property"),
    Int               = fnv1a("IntProperty"),
    Interface         = fnv1a("InterfaceProperty"),
    LazyObject        = fnv1a("LazyObjectProperty"),
    Map               = fnv1a("MapProperty"),
    MulticastDelegate = fnv1a("MulticastDelegateProperty"),
    MulticastInline   = fnv1a("MulticastInlineDelegateProperty"),
    MulticastSparse   = fnv1a("MulticastSparseDelegateProperty"),
    Name              = fnv1a("NameProperty"),
    Object            = fnv1a("ObjectProperty"),
    Set               = fnv1a("SetProperty"),
    SoftClass         = fnv1a("SoftClassProperty"),
    SoftObject        = fnv1a("SoftObjectProperty"),
    Str               = fnv1a("StrProperty"),
    Utf8Str           = fnv1a("Utf8StrProperty"),
    Struct            = fnv1a("StructProperty"),
    Text              = fnv1a("TextProperty"),
    UInt16            = fnv1a("UInt16Property"),
    UInt32            = fnv1a("UInt32Property"),
    UInt64            = fnv1a("UInt64Property"),
    WeakObject        = fnv1a("WeakObjectProperty"),
    Optional          = fnv1a("OptionalProperty"),

    VerseString   = fnv1a("VerseStringProperty"),
    VerseFunction = fnv1a("VerseFunctionProperty"),
    VerseDynamic  = fnv1a("VerseDynamicProperty"),

    Custom_FD      = fnv1a("CustomProperty_FD"),
    Custom_FE      = fnv1a("CustomProperty_FE"),
    GbxDefPtr      = fnv1a("GbxDefPtrProperty"),
    GameDataHandle = fnv1a("GameDataHandleProperty"),
  };

#pragma pack(push, 1)
  struct FPropertyTagData {
    // Meta
    std::string Name; // optional
    EPropertyTagType Type = EPropertyTagType::Unknown;

    std::string StructType;
    std::optional<FGuid> StructGuid;

    std::optional<bool> BoolValue;

    std::string EnumName;

    std::string InnerType;
    std::unique_ptr<FPropertyTagData> InnerTypeData;

    std::string ValueType;
    std::unique_ptr<FPropertyTagData> ValueTypeData;
  };


  struct FStructField {
    std::string Name;
    std::unique_ptr<FPropertyValue> Value;
  };

  struct IntPoint {
    int32_t x;
    int32_t y;
  };

  struct FStripDataFlags
  {

    uint8_t GlobalStripFlags;
    uint8_t ClassStripFlags;


    [[nodiscard]] bool isEditorDataStripped() const { return (GlobalStripFlags & 1) != 0;}
    [[nodiscard]] bool isAudioVisualDataStripped() const { return  (GlobalStripFlags & 2) != 0;}
    [[nodiscard]] bool isDataNeededForCookingStripped() const { return  (GlobalStripFlags & 4) != 0;}
    [[nodiscard]] bool isClassDataStripped(uint8_t flag) const { return (ClassStripFlags & flag) != 0;}
  };



  struct FPropertyValue {
    using Value = std::variant<
      std::monostate,
      IntPoint,
      bool,
      int8_t, int16_t, int32_t, int64_t,
      uint8_t, uint16_t, uint32_t, uint64_t,
      float, double,
      std::string,
      FName,
      FGuid,
      std::vector<FStructField>,
      std::vector<std::unique_ptr<FPropertyValue> >,
      std::unordered_map<
        std::unique_ptr<FPropertyValue>,
        std::unique_ptr<FPropertyValue>
      >
    >;

    Value Data;
  };

#pragma pack(pop)
}
