//
// Created by oleub on 20.12.25.
//

//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <memory>
#include <memory_resource>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "assets/material/f_material_override_nanite.hpp"
#include "assets/meshes/f_weighted_random_sampler.hpp"

#include "core/curves/f_compressed_rich_curve.hpp"
#include "core/curves/rich_curve.hpp"
#include "core/f_color.hpp"
#include "core/f_guid.hpp"
#include "core/f_linear_color.hpp"
#include "core/f_text.hpp"
#include "core/hash.hpp"
#include "core/math/f_box.hpp"
#include "core/math/f_quat.hpp"
#include "core/math/f_vector.hpp"
#include "core/name.hpp"
#include "core/per_quality_level_props.hpp"

#include "AssetFileParser.hpp"
#include "PropertyTracker.hpp"
#include "property_holder.hpp"
#include "sc/hash/hash.hpp"
#include "sc/localizations/magpie_string.hpp"
#include "sc/memory/monotonic_resource.hpp"
#include "soft_object.hpp"


namespace SAA {
  struct FPropertyValue;
  class Object;
  struct FPropertyTag;

  enum class ESatisfactoryStruct : sc::h64 {
    Unknown                               = 0,
    Guid                                  = "Guid"_h,
    IntPoint                              = "IntPoint"_h,
    Vector                                = "Vector"_h,
    Vector4d                              = "Vector4d"_h,
    Vector3f                              = "Vector3f"_h,
    Vector2f                              = "Vector2f"_h,
    Rotator                               = "Rotator"_h,
    IntVector2                            = "IntVector2"_h,
    Quat                                  = "Quat"_h,
    Box                                   = "Box"_h,
    Transform                             = "Transform"_h,
    KAggregateGeom                        = "KAggregateGeom"_h,
    KBoxElem                              = "KBoxElem"_h,
    KConvexElem                           = "KConvexElem"_h,
    BodyInstance                          = "BodyInstance"_h,
    CollisionResponse                     = "CollisionResponse"_h,
    ResponseChannel                       = "ResponseChannel"_h,
    StaticMaterial                        = "StaticMaterial"_h,
    MeshUVChannelInfo                     = "MeshUVChannelInfo"_h,
    BoxSphereBounds                       = "BoxSphereBounds"_h,
    LinearColor                           = "LinearColor"_h,
    ScalarParameterValue                  = "ScalarParameterValue"_h,
    VectorParameterValue                  = "VectorParameterValue"_h,
    TextureParameterValue                 = "TextureParameterValue"_h,
    MaterialParameterInfo                 = "MaterialParameterInfo"_h,
    MaterialInstanceBasePropertyOverrides = "MaterialInstanceBasePropertyOverrides"_h,
    MaterialTextureInfo                   = "MaterialTextureInfo"_h,
    StaticParameterSetRuntimeData         = "StaticParameterSetRuntimeData"_h,
    StaticSwitchParameter                 = "StaticSwitchParameter"_h,
    MaterialCachedParameterEntry          = "MaterialCachedParameterEntry"_h,
    Color                                 = "Color"_h,
    RichCurveKey                          = "RichCurveKey"_h,
    Vector4                               = "Vector4"_h,
    Box3f                                 = "Box3f"_h,
    DeprecateSlateVector2D                = "DeprecateSlateVector2D"_h,
    Vector2D                              = "Vector2D"_h,
    SkeletalMeshSamplingLODBuiltData      = "SkeletalMeshSamplingLODBuiltData"_h,
    PerPlatformFloat                      = "PerPlatformFloat"_h,
    KeyHandleMap                          = "KeyHandleMap"_h,
    CompressedRichCurve                   = "CompressedRichCurve"_h,
    IntVector                             = "IntVector"_h,
    MaterialOverrideNanite                = "MaterialOverrideNanite"_h,
    NiagaraUserRedirectionParameterStore  = "NiagaraUserRedirectionParameterStore"_h,
    VehicleInputRateConfig                = "VehicleInputRateConfig"_h,
    PerPlatformInt                        = "PerPlatformInt"_h,
    PerQualityLevelFloat                  = "PerQualityLevelFloat"_h,
    PerQualityLevelInt                    = "PerQualityLevelInt"_h,
    IntVector2D                           = "IntVector2D"_h,
    SoftClassPath                         = "SoftClassPath"_h,
    InventoryItem                         = "InventoryItem"_h,
    SoftObjectPath                        = "SoftObjectPath"_h,
    MovieSceneFrameRange                  = "MovieSceneFrameRange"_h,
    FontData                              = "FontData"_h,
    MovieSceneEvaluationFieldEntityTree   = "MovieSceneEvaluationFieldEntityTree"_h,
    FontCharacter                         = "FontCharacter"_h,
    FrameNumber                           = "FrameNumber"_h,
    MovieSceneFloatChannel                = "MovieSceneFloatChannel"_h,
    Box2f                                 = "Box2f"_h,
    MovieSceneTrackIdentifier             = "MovieSceneTrackIdentifier"_h,
    LevelSequenceObjectReferenceMap       = "LevelSequenceObjectReferenceMap"_h,
    SmartName                             = "SmartName"_h,
    DateTime                              = "DateTime"_h,
    FGServerCertificate                   = "FGServerCertificate"_h,
    PerPlatformFrameRate                  = "PerPlatformFrameRate"_h,
    ShaderValueTypeHandle                 = "ShaderValueTypeHandle"_h,

    // --- Niagara ---
    NiagaraVariable                      = "NiagaraVariable"_h,
    NiagaraVariableBase                  = "NiagaraVariableBase"_h,
    NiagaraVariableWithOffset            = "NiagaraVariableWithOffset"_h,
    NiagaraDataChannelVariable           = "NiagaraDataChannelVariable"_h,
    NiagaraDataInterfaceGPUParamInfo     = "NiagaraDataInterfaceGPUParamInfo"_h,
    NiagaraScript                        = "NiagaraScript"_h,
    NiagaraScriptExecutionParameterStore = "NiagaraScriptExecutionParameterStore"_h,

    // --- MovieScene / Sequencer ---
    MovieSceneSequenceID              = "MovieSceneSequenceID"_h,
    MovieSceneEvaluationKey           = "MovieSceneEvaluationKey"_h,
    MovieSceneEventParameters         = "MovieSceneEventParameters"_h,
    MovieSceneSegment                 = "MovieSceneSegment"_h,
    MovieSceneSegmentIdentifier       = "MovieSceneSegmentIdentifier"_h,
    MovieSceneTrackFieldData          = "MovieSceneTrackFieldData"_h,
    MovieSceneSubSequenceTree         = "MovieSceneSubSequenceTree"_h,
    MovieSceneSequenceInstanceDataPtr = "MovieSceneSequenceInstanceDataPtr"_h,
    MovieSceneEvalTemplatePtr         = "MovieSceneEvalTemplatePtr"_h,
    MovieSceneTrackImplementationPtr  = "MovieSceneTrackImplementationPtr"_h,
    SectionEvaluationDataTree         = "SectionEvaluationDataTree"_h,
    MovieSceneDoubleChannel           = "MovieSceneDoubleChannel"_h,
    MovieSceneTrackIdentifiers        = "MovieSceneTrackIdentifiers"_h,
    MovieSceneByteChannel             = "MovieSceneByteChannel"_h,
    GameplayTagContainer              = "GameplayTagContainer"_h,
    UniversalObjectLocatorFragment    = "UniversalObjectLocatorFragment"_h,
    InstancedStruct                   = "InstancedStruct"_h,
  };

  enum class EPropertyTagType : uint64_t {
    Unknown           = 0,
    None              = "None"_h,
    Array             = "ArrayProperty"_h,
    AssetObject       = "AssetObjectProperty"_h,
    AssetClass        = "AssetClassProperty"_h,
    Bool              = "BoolProperty"_h,
    Byte              = "ByteProperty"_h,
    Class             = "ClassProperty"_h,
    Delegate          = "DelegateProperty"_h,
    Double            = "DoubleProperty"_h,
    Enum              = "EnumProperty"_h,
    FieldPath         = "FieldPathProperty"_h,
    Float             = "FloatProperty"_h,
    Int16             = "Int16Property"_h,
    Int64             = "Int64Property"_h,
    Int8              = "Int8Property"_h,
    Int               = "IntProperty"_h,
    Interface         = "InterfaceProperty"_h,
    LazyObject        = "LazyObjectProperty"_h,
    Map               = "MapProperty"_h,
    MulticastDelegate = "MulticastDelegateProperty"_h,
    MulticastInline   = "MulticastInlineDelegateProperty"_h,
    MulticastSparse   = "MulticastSparseDelegateProperty"_h,
    Name              = "NameProperty"_h,
    Object            = "ObjectProperty"_h,
    Set               = "SetProperty"_h,
    SoftClass         = "SoftClassProperty"_h,
    SoftObject        = "SoftObjectProperty"_h,
    Str               = "StrProperty"_h,
    Utf8Str           = "Utf8StrProperty"_h,
    Struct            = "StructProperty"_h,
    Text              = "TextProperty"_h,
    UInt16            = "UInt16Property"_h,
    UInt32            = "UInt32Property"_h,
    UInt64            = "UInt64Property"_h,
    WeakObject        = "WeakObjectProperty"_h,
    Optional          = "OptionalProperty"_h,

    VerseString   = "VerseStringProperty"_h,
    VerseFunction = "VerseFunctionProperty"_h,
    VerseDynamic  = "VerseDynamicProperty"_h,

    Custom_FD      = "CustomProperty_FD"_h,
    Custom_FE      = "CustomProperty_FE"_h,
    GbxDefPtr      = "GbxDefPtrProperty"_h,
    GameDataHandle = "GameDataHandleProperty"_h,
  };

  struct alignas(8) FPropertyTypeNameNode {
    FName Name;
    int32_t innerCount;
  };

  inline int32_t countSubtreeNodes(std::span<FPropertyTypeNameNode> s) {
    if (s.empty())
      return 0;
    int32_t remaining = 1;
    int32_t count     = 0;
    while (remaining > 0 && static_cast<size_t>(count) < s.size()) {
      remaining += (s[count].innerCount - 1);
      count++;
    }
    return count;
  }

  inline std::span<FPropertyTypeNameNode> getParameter(std::span<FPropertyTypeNameNode> nodes, int32_t paramIndex) {
    // TODO: [[likely]] steht hier am Fehlerpfad (leer oder Index ausserhalb). Vermutlich ist [[unlikely]] gemeint.
    if (nodes.empty() || paramIndex < 0 || paramIndex >= nodes[0].innerCount) [[likely]] {
      return {};
    }

    int32_t offset = 1;
    for (int32_t i = 0; i < paramIndex; ++i) {
      offset += countSubtreeNodes(nodes.subspan(offset));
    }

    int32_t length = countSubtreeNodes(nodes.subspan(offset));

    return nodes.subspan(offset, length);
  }


  struct FPropertyTagData {
    FName Name; // optional
    EPropertyTagType Type = EPropertyTagType::Unknown;

    FName structName;
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

  enum class EPropertyTagSerializeType : uint8_t { Unknown, Skipped, Property, BinaryOrNative };

  enum class EPropertyTagFlags : uint8_t {
    None                       = 0x00,
    HasArrayIndex              = 0x01,
    HasPropertyGuid            = 0x02,
    HasPropertyExtensions      = 0x04,
    HasBinaryOrNativeSerialize = 0x08,
    BoolTrue                   = 0x10,
    SkippedSerialize           = 0x20
  };

  // Bitweise OR
  inline EPropertyTagFlags operator|(EPropertyTagFlags lhs, EPropertyTagFlags rhs) {
    return static_cast<EPropertyTagFlags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
  }

  // Bitweise AND
  inline EPropertyTagFlags operator&(EPropertyTagFlags lhs, EPropertyTagFlags rhs) {
    return static_cast<EPropertyTagFlags>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
  }

  // Bitweise XOR
  inline EPropertyTagFlags operator^(EPropertyTagFlags lhs, EPropertyTagFlags rhs) {
    return static_cast<EPropertyTagFlags>(static_cast<uint8_t>(lhs) ^ static_cast<uint8_t>(rhs));
  }

  // Bitweise NOT
  inline EPropertyTagFlags operator~(EPropertyTagFlags arg) {
    return static_cast<EPropertyTagFlags>(~static_cast<uint8_t>(arg));
  }

  // Compound Assignments
  inline EPropertyTagFlags &operator|=(EPropertyTagFlags &lhs, EPropertyTagFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  inline EPropertyTagFlags &operator&=(EPropertyTagFlags &lhs, EPropertyTagFlags rhs) {
    lhs = lhs & rhs;
    return lhs;
  }

  inline bool hasFlag(EPropertyTagFlags value, EPropertyTagFlags flag) {
    return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) == static_cast<uint8_t>(flag);
  }

  inline bool anyFlag(EPropertyTagFlags value) { return value != EPropertyTagFlags::None; }

  enum class EPropertyTagExtension : uint8_t {
    NoExtension         = 0x00,
    ReserveForFutureUse = 0x01, // Can be use to add a next group of extension

    ////////////////////////////////////////////////
    // First extension group
    OverridableInformation = 0x02,

    //
    // Add more extension for the first group here
    //
  };

  inline bool hasFlag(EPropertyTagExtension value, EPropertyTagExtension flag) {
    return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) == static_cast<uint8_t>(flag);
  }


  struct FPropertyValue {
    using Value = std::variant<std::monostate, IntPoint, bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t,
                               uint32_t, uint64_t, float, double, std::string_view, Object *,

                               FMaterialOverrideNanite, FPerQualityLevelFloat, FPerQualityLevelInt,

                               FName, FGuid, SoftObj,

                               FVector, FVector3F, FVector4D, FVector2F, FVector2D, FIntVector, FIntVector2, FRotator,

                               FQuat, FBox, FBox2f, FBox3f, FLinearColor, sc::MagpieString, FColor, PropertyHolder,
                               FRichCurveKey, FCompressedRichCurve,

                               std::pmr::vector<FPropertyValue>,
                               std::pmr::vector<std::pair<FPropertyValue, FPropertyValue>>, FWeightedRandomSampler>;
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

    // TODO: isIndexed() und serializeType() vergleichen die Flags mit >= als Zahl statt per hasFlag() einzelne Bits
    // zu pruefen. BoolTrue (0x10) allein ist >= HasBinaryOrNativeSerialize (0x08), ein wahres Bool-Property wird also
    // als BinaryOrNative eingestuft, und jedes gesetzte Flag macht isIndexed() wahr.
    [[nodiscard]] bool isIndexed() const { return Flags >= EPropertyTagFlags::HasArrayIndex || ArrayIndex > 0; }

    [[nodiscard]] EPropertyTagSerializeType serializeType() const {
      if (Flags >= EPropertyTagFlags::SkippedSerialize)
        return EPropertyTagSerializeType::Skipped;
      if (Flags >= EPropertyTagFlags::HasBinaryOrNativeSerialize)
        return EPropertyTagSerializeType::BinaryOrNative;
      return EPropertyTagSerializeType::Property;
    }
  };
#ifndef NDEBUG
  struct PropertyInfo {
    FPropertyTag tag;
    size_t count = 0;
  };
#endif

  template<typename T>
  std::optional<T> PropertyHolder::get(uint64_t hash) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties->find(hash);
    if (it == m_properties->end())
      return std::nullopt;
    return std::get<T>(it->second.Tag.data);
  }

  template<typename T, typename Func>
  std::optional<T> PropertyHolder::get(uint64_t hash, Func &&processor) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties->find(hash);
    if (it == m_properties->end())
      return std::nullopt;

    if (const auto *prop = std::get_if<PropertyHolder>(&it->second.Tag.data)) {
      return processor(prop);
    }

    return std::nullopt;
  }

  // TODO: Beide getArray()-Varianten legen bei jedem Aufruf einen neuen Vektor im Speicher des Parsers an, der erst
  // mit dessen reset() freigegeben wird. Wiederholte Aufrufe fuer dasselbe Property lassen ihn wachsen.
  template<typename T>
  std::span<T> PropertyHolder::getArray(uint64_t hash) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties->find(hash);

    if (it == m_properties->end())
      return {};

    const auto &val = std::get<std::pmr::vector<FPropertyValue>>(it->second.Tag.data);

    auto *vector = m_owner->storage()->makePmr<std::pmr::vector<T>>();
    vector->reserve(val.size());

    for (const auto &[data]: val) {
      vector->emplace_back(std::get<T>(data));
    }
    return *vector;
  }


  template<typename T, typename Func>
  std::span<T> PropertyHolder::getArray(uint64_t hash, Func &&processor) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties->find(hash);
    if (it == m_properties->end())
      return {};

    const auto &val = std::get<std::pmr::vector<FPropertyValue>>(it->second.Tag.data);

    auto *vector = m_owner->storage()->makePmr<std::pmr::vector<T>>();
    vector->reserve(val.size());

    for (const auto &[data]: val) {
      vector->emplace_back(processor(std::get<PropertyHolder>(data)));
    }
    return *vector;
  }
} // namespace SAA
