//
// Created by oleub on 20.12.25.
//

//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "property_holder.hpp"

#include "assets/material/f_material_override_nanite.hpp"
#include "assets/meshes/f_weighted_random_sampler.hpp"

#include "core/f_color.hpp"
#include "core/f_guid.hpp"
#include "core/f_linear_color.hpp"
#include "core/f_text.hpp"
#include "core/hash.hpp"
#include "core/name.hpp"
#include "core/per_quality_level_props.hpp"
#include "core/curves/f_compressed_rich_curve.hpp"
#include "core/curves/rich_curve.hpp"
#include "core/math/f_box.hpp"
#include "core/math/f_quat.hpp"
#include "core/math/f_vector.hpp"

namespace SAP {
  struct FPropertyValue;
  class Object;
  struct FPropertyTag;

  enum class ESatisfactoryStruct : uint32_t {
    Unknown                               = 0,
    Guid                                  = fnv1a("Guid"),
    IntPoint                              = fnv1a("IntPoint"),
    Vector                                = fnv1a("Vector"),
    Vector4d                              = fnv1a("Vector4d"),
    Vector3f                              = fnv1a("Vector3f"),
    Rotator                               = fnv1a("Rotator"),
    Quat                                  = fnv1a("Quat"),
    Box                                   = fnv1a("Box"),
    Transform                             = fnv1a("Transform"),
    KAggregateGeom                        = fnv1a("KAggregateGeom"),
    KBoxElem                              = fnv1a("KBoxElem"),
    KConvexElem                           = fnv1a("KConvexElem"),
    BodyInstance                          = fnv1a("BodyInstance"),
    CollisionResponse                     = fnv1a("CollisionResponse"),
    ResponseChannel                       = fnv1a("ResponseChannel"),
    StaticMaterial                        = fnv1a("StaticMaterial"),
    MeshUVChannelInfo                     = fnv1a("MeshUVChannelInfo"),
    BoxSphereBounds                       = fnv1a("BoxSphereBounds"),
    LinearColor                           = fnv1a("LinearColor"),
    ScalarParameterValue                  = fnv1a("ScalarParameterValue"),
    VectorParameterValue                  = fnv1a("VectorParameterValue"),
    TextureParameterValue                 = fnv1a("TextureParameterValue"),
    MaterialParameterInfo                 = fnv1a("MaterialParameterInfo"),
    MaterialInstanceBasePropertyOverrides = fnv1a("MaterialInstanceBasePropertyOverrides"),
    MaterialTextureInfo                   = fnv1a("MaterialTextureInfo"),
    StaticParameterSetRuntimeData         = fnv1a("StaticParameterSetRuntimeData"),
    StaticSwitchParameter                 = fnv1a("StaticSwitchParameter"),
    MaterialCachedParameterEntry          = fnv1a("MaterialCachedParameterEntry"),
    Color                                 = fnv1a("Color"),
    RichCurveKey                          = fnv1a("RichCurveKey"),
    Vector4                               = fnv1a("Vector4"),
    DeprecateSlateVector2D                = fnv1a("DeprecateSlateVector2D"),
    Vector2D                              = fnv1a("Vector2D"),
    SkeletalMeshSamplingLODBuiltData      = fnv1a("SkeletalMeshSamplingLODBuiltData"),
    PerPlatformFloat                      = fnv1a("PerPlatformFloat"),
    KeyHandleMap                          = fnv1a("KeyHandleMap"),
    CompressedRichCurve                   = fnv1a("CompressedRichCurve"),
    IntVector                             = fnv1a("IntVector"),
    MaterialOverrideNanite                = fnv1a("MaterialOverrideNanite"),
    NiagaraUserRedirectionParameterStore  = fnv1a("NiagaraUserRedirectionParameterStore"),
    VehicleInputRateConfig                = fnv1a("VehicleInputRateConfig"),
    PerPlatformInt                        = fnv1a("PerPlatformInt"),
    PerQualityLevelFloat                  = fnv1a("PerQualityLevelFloat"),
    PerQualityLevelInt                    = fnv1a("PerQualityLevelInt"),
    IntVector2D                           = fnv1a("IntVector2D"),
    SoftClassPath                         = fnv1a("SoftClassPath"),
    InventoryItem                         = fnv1a("InventoryItem"),
    SoftObjectPath                        = fnv1a("SoftObjectPath"),
    MovieSceneFrameRange                  = fnv1a("MovieSceneFrameRange"),
    FontData                              = fnv1a("FontData"),
    FontCharacter
  };

  inline const char *to_string(ESatisfactoryStruct s) {
    switch (s) {
      case ESatisfactoryStruct::Unknown: return "Unknown";

      case ESatisfactoryStruct::Guid: return "Guid";
      case ESatisfactoryStruct::IntPoint: return "IntPoint";
      case ESatisfactoryStruct::Vector: return "Vector";
      case ESatisfactoryStruct::Vector4d: return "Vector4d";
      case ESatisfactoryStruct::Rotator: return "Rotator";
      case ESatisfactoryStruct::Box: return "Box";
      case ESatisfactoryStruct::Quat: return "Quat";
      case ESatisfactoryStruct::Transform: return "Transform";
      case ESatisfactoryStruct::KAggregateGeom: return "KAggregateGeom";
      case ESatisfactoryStruct::KBoxElem: return "KBoxElem";
      case ESatisfactoryStruct::KConvexElem: return "KConvexElem";
      case ESatisfactoryStruct::BodyInstance: return "BodyInstance";
      case ESatisfactoryStruct::CollisionResponse: return "CollisionResponse";
      case ESatisfactoryStruct::ResponseChannel: return "ResponseChannel";
      case ESatisfactoryStruct::StaticMaterial: return "StaticMaterial";
      case ESatisfactoryStruct::BoxSphereBounds: return "BoxSphereBounds";
      case ESatisfactoryStruct::MeshUVChannelInfo: return "MeshUVChannelInfo";
      case ESatisfactoryStruct::LinearColor: return "LinearColor";
      case ESatisfactoryStruct::ScalarParameterValue: return "ScalarParameterValue";
      case ESatisfactoryStruct::VectorParameterValue: return "VectorParameterValue";
      case ESatisfactoryStruct::TextureParameterValue: return "TextureParameterValue";
      case ESatisfactoryStruct::MaterialParameterInfo: return "MaterialParameterInfo";
      case ESatisfactoryStruct::MaterialInstanceBasePropertyOverrides: return "MaterialInstanceBasePropertyOverrides";
      case ESatisfactoryStruct::MaterialTextureInfo: return "MaterialTextureInfo";
      case ESatisfactoryStruct::StaticParameterSetRuntimeData: return "StaticParameterSetRuntimeData";
      case ESatisfactoryStruct::StaticSwitchParameter: return "StaticSwitchParameter";
      case ESatisfactoryStruct::MaterialCachedParameterEntry: return "MaterialCachedParameterEntry";
      case ESatisfactoryStruct::Color: return "Color";
      case ESatisfactoryStruct::RichCurveKey: return "RichCurveKey";
      case ESatisfactoryStruct::Vector4: return "Vector4";
      case ESatisfactoryStruct::DeprecateSlateVector2D: return "DeprecateSlateVector2D";
      case ESatisfactoryStruct::Vector2D: return "Vector2D";

      default: return "InvalidESatisfactoryStruct";
    }
  }

  enum class EPropertyTagType : uint32_t {
    Unknown           = 0,
    None              = fnv1a("None"),
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

  inline std::string to_string(EPropertyTagType type) {
    switch (type) {
      case EPropertyTagType::Unknown: return "Unknown";
      case EPropertyTagType::Array: return "ArrayProperty";
      case EPropertyTagType::AssetObject: return "AssetObjectProperty";
      case EPropertyTagType::AssetClass: return "AssetClassProperty";
      case EPropertyTagType::Bool: return "BoolProperty";
      case EPropertyTagType::Byte: return "ByteProperty";
      case EPropertyTagType::Class: return "ClassProperty";
      case EPropertyTagType::Delegate: return "DelegateProperty";
      case EPropertyTagType::Double: return "DoubleProperty";
      case EPropertyTagType::Enum: return "EnumProperty";
      case EPropertyTagType::FieldPath: return "FieldPathProperty";
      case EPropertyTagType::Float: return "FloatProperty";
      case EPropertyTagType::Int16: return "Int16Property";
      case EPropertyTagType::Int64: return "Int64Property";
      case EPropertyTagType::Int8: return "Int8Property";
      case EPropertyTagType::Int: return "IntProperty";
      case EPropertyTagType::Interface: return "InterfaceProperty";
      case EPropertyTagType::LazyObject: return "LazyObjectProperty";
      case EPropertyTagType::Map: return "MapProperty";
      case EPropertyTagType::MulticastDelegate: return "MulticastDelegateProperty";
      case EPropertyTagType::MulticastInline: return "MulticastInlineDelegateProperty";
      case EPropertyTagType::MulticastSparse: return "MulticastSparseDelegateProperty";
      case EPropertyTagType::Name: return "NameProperty";
      case EPropertyTagType::Object: return "ObjectProperty";
      case EPropertyTagType::Set: return "SetProperty";
      case EPropertyTagType::SoftClass: return "SoftClassProperty";
      case EPropertyTagType::SoftObject: return "SoftObjectProperty";
      case EPropertyTagType::Str: return "StrProperty";
      case EPropertyTagType::Utf8Str: return "Utf8StrProperty";
      case EPropertyTagType::Struct: return "StructProperty";
      case EPropertyTagType::Text: return "TextProperty";
      case EPropertyTagType::UInt16: return "UInt16Property";
      case EPropertyTagType::UInt32: return "UInt32Property";
      case EPropertyTagType::UInt64: return "UInt64Property";
      case EPropertyTagType::WeakObject: return "WeakObjectProperty";
      case EPropertyTagType::Optional: return "OptionalProperty";

      case EPropertyTagType::VerseString: return "VerseStringProperty";
      case EPropertyTagType::VerseFunction: return "VerseFunctionProperty";
      case EPropertyTagType::VerseDynamic: return "VerseDynamicProperty";

      case EPropertyTagType::Custom_FD: return "CustomProperty_FD";
      case EPropertyTagType::Custom_FE: return "CustomProperty_FE";
      case EPropertyTagType::GbxDefPtr: return "GbxDefPtrProperty";
      case EPropertyTagType::GameDataHandle: return "GameDataHandleProperty";
      case EPropertyTagType::None: return "None";
    }
    return "<invalid>";
  }

  struct FPropertyTagData {
    FName Name; // optional
    EPropertyTagType Type = EPropertyTagType::Unknown;

    ESatisfactoryStruct StructType;
    FGuid StructGuid;

    std::optional<bool> BoolValue;

    FName EnumName;

    EPropertyTagType InnerType;
    FPropertyTagData *InnerTypeData;

    EPropertyTagType ValueType;
    FPropertyTagData *ValueTypeData;
  };

  struct FStructField {
    std::string Name;
    std::unique_ptr<FPropertyValue> Value;
  };

  struct IntPoint {
    int32_t x;
    int32_t y;
  };

  struct FStripDataFlags {
    uint8_t GlobalStripFlags;
    uint8_t ClassStripFlags;

    [[nodiscard]] bool isEditorDataStripped() const { return (GlobalStripFlags & 1) != 0; }
    [[nodiscard]] bool isAudioVisualDataStripped() const { return (GlobalStripFlags & 2) != 0; }
    [[nodiscard]] bool isDataNeededForCookingStripped() const { return (GlobalStripFlags & 4) != 0; }
    [[nodiscard]] bool isClassDataStripped(uint8_t flag) const { return (ClassStripFlags & flag) != 0; }
  };

  enum class EPropertyTagSerializeType : uint8_t {
    Unknown,
    Skipped,
    Property,
    BinaryOrNative
  };

  enum class EPropertyTagFlags : uint8_t {
    None                       = 0x00,
    HasArrayIndex              = 0x01,
    HasPropertyGuid            = 0x02,
    HasPropertyExtensions      = 0x04,
    HasBinaryOrNativeSerialize = 0x08,
    BoolTrue                   = 0x10,
    SkippedSerialize           = 0x20
  };

  struct FPropertyValue {
    using Value = std::variant<
      std::monostate,
      IntPoint,
      bool,
      int8_t, int16_t, int32_t, int64_t,
      uint8_t, uint16_t, uint32_t, uint64_t,
      float, double,
      std::string_view,
      std::weak_ptr<Object>,

      FMaterialOverrideNanite,
      FPerQualityLevelFloat,
      FPerQualityLevelInt,

      FName,
      FGuid,

      FVector,
      FVector_f,
      FVector4d,
      FVector2F,
       FVector2D,
      FIntVector,
      FIntVector2D,
      FRotator,

      FQuat,
      FBox,
      FLinearColor,
      FText,
      FColor,
      PropertyHolder,
      FRichCurveKey,
      FCompressedRichCurve,

      std::vector<FPropertyValue>,
      std::vector<std::pair<FPropertyValue, FPropertyValue> >,
      FWeightedRandomSampler
    >;
    Value data;
  };

  struct FPropertyTag {
    FName Name;
    FName PropertyTypeName;
    EPropertyTagType PropertyType{};

    uint32_t Size{};
    uint32_t ArrayIndex{};
    std::optional<uint32_t> ArraySize;

    FGuid PropertyGuid{};
    EPropertyTagFlags Flags = EPropertyTagFlags::None;

    FPropertyTagData *TagData{};
    FPropertyValue Tag;

    [[nodiscard]] bool isIndexed() const { return Flags >= EPropertyTagFlags::HasArrayIndex || ArrayIndex > 0; }

    [[nodiscard]] EPropertyTagSerializeType serializeType() const {
      if (Flags >= EPropertyTagFlags::SkippedSerialize)
        return EPropertyTagSerializeType::Skipped;
      if (Flags >= EPropertyTagFlags::HasBinaryOrNativeSerialize)
        return EPropertyTagSerializeType::BinaryOrNative;
      return EPropertyTagSerializeType::Property;
    }
  };
}
