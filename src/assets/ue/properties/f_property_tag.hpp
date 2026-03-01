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

#include "property_holder.hpp"
#include "AssetFileParser.hpp"
#include "PropertyTracker.hpp"
#include "soft_object.hpp"


namespace SAP {
  struct FPropertyValue;
  class Object;
  struct FPropertyTag;

  enum class ESatisfactoryStruct : uint32_t {
    Unknown = 0,
    Guid = fnv1a("Guid"),
    IntPoint = fnv1a("IntPoint"),
    Vector = fnv1a("Vector"),
    Vector4d = fnv1a("Vector4d"),
    Vector3f = fnv1a("Vector3f"),
    Vector2f = fnv1a("Vector2f"),
    Rotator = fnv1a("Rotator"),
    Quat = fnv1a("Quat"),
    Box = fnv1a("Box"),
    Transform = fnv1a("Transform"),
    KAggregateGeom = fnv1a("KAggregateGeom"),
    KBoxElem = fnv1a("KBoxElem"),
    KConvexElem = fnv1a("KConvexElem"),
    BodyInstance = fnv1a("BodyInstance"),
    CollisionResponse = fnv1a("CollisionResponse"),
    ResponseChannel = fnv1a("ResponseChannel"),
    StaticMaterial = fnv1a("StaticMaterial"),
    MeshUVChannelInfo = fnv1a("MeshUVChannelInfo"),
    BoxSphereBounds = fnv1a("BoxSphereBounds"),
    LinearColor = fnv1a("LinearColor"),
    ScalarParameterValue = fnv1a("ScalarParameterValue"),
    VectorParameterValue = fnv1a("VectorParameterValue"),
    TextureParameterValue = fnv1a("TextureParameterValue"),
    MaterialParameterInfo = fnv1a("MaterialParameterInfo"),
    MaterialInstanceBasePropertyOverrides = fnv1a("MaterialInstanceBasePropertyOverrides"),
    MaterialTextureInfo = fnv1a("MaterialTextureInfo"),
    StaticParameterSetRuntimeData = fnv1a("StaticParameterSetRuntimeData"),
    StaticSwitchParameter = fnv1a("StaticSwitchParameter"),
    MaterialCachedParameterEntry = fnv1a("MaterialCachedParameterEntry"),
    Color = fnv1a("Color"),
    RichCurveKey = fnv1a("RichCurveKey"),
    Vector4 = fnv1a("Vector4"),
    DeprecateSlateVector2D = fnv1a("DeprecateSlateVector2D"),
    Vector2D = fnv1a("Vector2D"),
    SkeletalMeshSamplingLODBuiltData = fnv1a("SkeletalMeshSamplingLODBuiltData"),
    PerPlatformFloat = fnv1a("PerPlatformFloat"),
    KeyHandleMap = fnv1a("KeyHandleMap"),
    CompressedRichCurve = fnv1a("CompressedRichCurve"),
    IntVector = fnv1a("IntVector"),
    MaterialOverrideNanite = fnv1a("MaterialOverrideNanite"),
    NiagaraUserRedirectionParameterStore = fnv1a("NiagaraUserRedirectionParameterStore"),
    VehicleInputRateConfig = fnv1a("VehicleInputRateConfig"),
    PerPlatformInt = fnv1a("PerPlatformInt"),
    PerQualityLevelFloat = fnv1a("PerQualityLevelFloat"),
    PerQualityLevelInt = fnv1a("PerQualityLevelInt"),
    IntVector2D = fnv1a("IntVector2D"),
    SoftClassPath = fnv1a("SoftClassPath"),
    InventoryItem = fnv1a("InventoryItem"),
    SoftObjectPath = fnv1a("SoftObjectPath"),
    MovieSceneFrameRange = fnv1a("MovieSceneFrameRange"),
    FontData = fnv1a("FontData"),
    MovieSceneEvaluationFieldEntityTree = fnv1a("MovieSceneEvaluationFieldEntityTree"),
    FontCharacter = fnv1a("FontCharacter"),
    FrameNumber = fnv1a("FrameNumber"),
    MovieSceneFloatChannel = fnv1a("MovieSceneFloatChannel"),
    Box2f = fnv1a("Box2f"),
    MovieSceneTrackIdentifier = fnv1a("MovieSceneTrackIdentifier"),
    LevelSequenceObjectReferenceMap = fnv1a("LevelSequenceObjectReferenceMap"),
    SmartName = fnv1a("SmartName"),
    DateTime = fnv1a("DateTime"),
    FGServerCertificate = fnv1a("FGServerCertificate"),
    PerPlatformFrameRate = fnv1a("PerPlatformFrameRate"),
    ShaderValueTypeHandle = fnv1a("ShaderValueTypeHandle"),

    // --- Niagara ---
    NiagaraVariable = fnv1a("NiagaraVariable"),
    NiagaraVariableBase = fnv1a("NiagaraVariableBase"),
    NiagaraVariableWithOffset = fnv1a("NiagaraVariableWithOffset"),
    NiagaraDataChannelVariable = fnv1a("NiagaraDataChannelVariable"),
    NiagaraDataInterfaceGPUParamInfo = fnv1a("NiagaraDataInterfaceGPUParamInfo"),
    NiagaraScript = fnv1a("NiagaraScript"),
    NiagaraScriptExecutionParameterStore = fnv1a("NiagaraScriptExecutionParameterStore"),

    // --- MovieScene / Sequencer ---
    MovieSceneSequenceID = fnv1a("MovieSceneSequenceID"),
    MovieSceneEvaluationKey = fnv1a("MovieSceneEvaluationKey"),
    MovieSceneEventParameters = fnv1a("MovieSceneEventParameters"),
    MovieSceneSegment = fnv1a("MovieSceneSegment"),
    MovieSceneSegmentIdentifier = fnv1a("MovieSceneSegmentIdentifier"),
    MovieSceneTrackFieldData = fnv1a("MovieSceneTrackFieldData"),
    MovieSceneSubSequenceTree = fnv1a("MovieSceneSubSequenceTree"),
    MovieSceneSequenceInstanceDataPtr = fnv1a("MovieSceneSequenceInstanceDataPtr"),
    MovieSceneEvalTemplatePtr = fnv1a("MovieSceneEvalTemplatePtr"),
    MovieSceneTrackImplementationPtr = fnv1a("MovieSceneTrackImplementationPtr"),
    SectionEvaluationDataTree = fnv1a("SectionEvaluationDataTree"),
    MovieSceneDoubleChannel = fnv1a("MovieSceneDoubleChannel"),
    MovieSceneTrackIdentifiers = fnv1a("MovieSceneTrackIdentifiers"),
    MovieSceneByteChannel = fnv1a("MovieSceneByteChannel"),
  };


  enum class EPropertyTagType : uint32_t {
    Unknown = 0,
    None = fnv1a("None"),
    Array = fnv1a("ArrayProperty"),
    AssetObject = fnv1a("AssetObjectProperty"),
    AssetClass = fnv1a("AssetClassProperty"),
    Bool = fnv1a("BoolProperty"),
    Byte = fnv1a("ByteProperty"),
    Class = fnv1a("ClassProperty"),
    Delegate = fnv1a("DelegateProperty"),
    Double = fnv1a("DoubleProperty"),
    Enum = fnv1a("EnumProperty"),
    FieldPath = fnv1a("FieldPathProperty"),
    Float = fnv1a("FloatProperty"),
    Int16 = fnv1a("Int16Property"),
    Int64 = fnv1a("Int64Property"),
    Int8 = fnv1a("Int8Property"),
    Int = fnv1a("IntProperty"),
    Interface = fnv1a("InterfaceProperty"),
    LazyObject = fnv1a("LazyObjectProperty"),
    Map = fnv1a("MapProperty"),
    MulticastDelegate = fnv1a("MulticastDelegateProperty"),
    MulticastInline = fnv1a("MulticastInlineDelegateProperty"),
    MulticastSparse = fnv1a("MulticastSparseDelegateProperty"),
    Name = fnv1a("NameProperty"),
    Object = fnv1a("ObjectProperty"),
    Set = fnv1a("SetProperty"),
    SoftClass = fnv1a("SoftClassProperty"),
    SoftObject = fnv1a("SoftObjectProperty"),
    Str = fnv1a("StrProperty"),
    Utf8Str = fnv1a("Utf8StrProperty"),
    Struct = fnv1a("StructProperty"),
    Text = fnv1a("TextProperty"),
    UInt16 = fnv1a("UInt16Property"),
    UInt32 = fnv1a("UInt32Property"),
    UInt64 = fnv1a("UInt64Property"),
    WeakObject = fnv1a("WeakObjectProperty"),
    Optional = fnv1a("OptionalProperty"),

    VerseString = fnv1a("VerseStringProperty"),
    VerseFunction = fnv1a("VerseFunctionProperty"),
    VerseDynamic = fnv1a("VerseDynamicProperty"),

    Custom_FD = fnv1a("CustomProperty_FD"),
    Custom_FE = fnv1a("CustomProperty_FE"),
    GbxDefPtr = fnv1a("GbxDefPtrProperty"),
    GameDataHandle = fnv1a("GameDataHandleProperty"),
  };

  struct FPropertyTypeNameNode {
    FName Name;
    int32_t innerCount;
  };

  inline int32_t countSubtreeNodes(std::span<FPropertyTypeNameNode> s) {
    if (s.empty()) return 0;
    int32_t remaining = 1;
    int32_t count = 0;
    while (remaining > 0 && count < s.size()) {
      remaining += (s[count].innerCount - 1);
      count++;
    }
    return count;
  }

  inline std::span<FPropertyTypeNameNode> getParameter(std::span<FPropertyTypeNameNode> nodes, int32_t paramIndex) {
    if (nodes.empty() || paramIndex < 0 || paramIndex >= nodes[0].innerCount) {
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
    None = 0x00,
    HasArrayIndex = 0x01,
    HasPropertyGuid = 0x02,
    HasPropertyExtensions = 0x04,
    HasBinaryOrNativeSerialize = 0x08,
    BoolTrue = 0x10,
    SkippedSerialize = 0x20
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
  inline EPropertyTagFlags& operator|=(EPropertyTagFlags& lhs, EPropertyTagFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  inline EPropertyTagFlags& operator&=(EPropertyTagFlags& lhs, EPropertyTagFlags rhs) {
    lhs = lhs & rhs;
    return lhs;
  }

  inline bool hasFlag(EPropertyTagFlags value, EPropertyTagFlags flag) {
    return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) == static_cast<uint8_t>(flag);
  }

  inline bool anyFlag(EPropertyTagFlags value) {
    return value != EPropertyTagFlags::None;
  }

  enum class  EPropertyTagExtension : uint8_t
  {
    NoExtension					= 0x00,
    ReserveForFutureUse			= 0x01, // Can be use to add a next group of extension

    ////////////////////////////////////////////////
    // First extension group
    OverridableInformation		= 0x02,

    //
    // Add more extension for the first group here
    //
};

  inline bool hasFlag(EPropertyTagExtension value, EPropertyTagExtension flag) {
    return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) == static_cast<uint8_t>(flag);
  }


  struct FPropertyValue {
    using Value = std::variant<
      std::monostate, IntPoint, bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t,
      float,
      double, std::string_view, Object *,

      FMaterialOverrideNanite, FPerQualityLevelFloat, FPerQualityLevelInt,

      FName, FGuid, SoftObj,

      FVector, FVector_f, FVector4d, FVector2F, FVector2D, FIntVector, FIntVector2D, FRotator,

      FQuat, FBox, FBox2f, FLinearColor, FText, FColor, PropertyHolder, FRichCurveKey, FCompressedRichCurve,

      std::pmr::vector<FPropertyValue>, std::pmr::vector<std::pair<FPropertyValue, FPropertyValue> >,
      FWeightedRandomSampler>;
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
#ifndef NDEBUG
  struct PropertyInfo {
    FPropertyTag tag;
    size_t count = 0;
  };
#endif

  template<typename T>
  std::optional<T> PropertyHolder::get(uint32_t hash) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties.find(hash);
    if (it == m_properties.end())
      return std::nullopt;
    return std::get<T>(it->second.Tag.data);
  }

  template<typename T, typename Func>
  std::optional<T> PropertyHolder::get(uint32_t hash, Func &&processor) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties.find(hash);
    if (it == m_properties.end())
      return std::nullopt;

    if (const auto *prop = std::get_if<PropertyHolder>(&it->second.Tag.data)) {
      return processor(prop);
    }

    return std::nullopt;
  }

  template<typename T>
  std::span<T> PropertyHolder::getArray(uint32_t hash) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties.find(hash);

    if (it == m_properties.end()) return {};

    const auto &val = std::get<std::pmr::vector<FPropertyValue> >(it->second.Tag.data);

    auto *vector = m_owner->storage()->make<std::pmr::vector<T> >(m_owner->storage()->memoryResource());
    vector->reserve(val.size());

    for (const auto &[data]: val) {
      vector->emplace_back(std::get<T>(data));
    }
    return *vector;
  }

  template<typename T, typename Func>
  std::span<T> PropertyHolder::getArray(uint32_t hash, Func &&processor) const {
    DBG_MARK_HANDLED(m_className, hash);
    auto it = m_properties.find(hash);
    if (it == m_properties.end()) return {};

    const auto &val = std::get<std::pmr::vector<FPropertyValue> >(it->second.Tag.data);

    auto *vector = m_owner->storage()->make<std::pmr::vector<T> >(m_owner->storage()->memoryResource());
    vector->reserve(val.size());

    for (const auto &[data]: val) {
      vector->emplace_back(processor(std::get<PropertyHolder>(data)));
    }
    return *vector;
  }
} // namespace SAP
