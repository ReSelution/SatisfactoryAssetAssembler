//
// Created by oleub on 23.02.26.
//

#include "AssetParser.hpp"
#include "property_parser.hpp"
#include "core/EClassSerializationControlExtension.hpp"
#include "io/packages/f_byte_bulk_data.hpp"


namespace SAA {
  template<>
  void PP6::parse(PropertyHolder *holder, ByteCursor &cursor, SC::ChaosBumpArena *storage) {
    auto controlByte = cursor.read<EClassSerializationControlExtension>();
    if (hasFlag(controlByte, EClassSerializationControlExtension::OverridableSerializationInformation)) {
      auto op = cursor.read<uint8_t>();
    }

    while (true) {
      auto tag = parsePropertyTag(cursor, storage);
      if (tag.Name.isNone()) {
        return;
      }
      auto final = cursor.tell() + tag.Size;
      tag.Tag = parsePropertyValue(cursor, storage, tag, ReadType::NORMAL, tag.Size);
      cursor = final;
      holder->m_properties->emplace(SC::hash(tag.Name.text()), std::move(tag));
    }
  }

  template<>
  FPropertyTag PP6::parsePropertyTag(ByteCursor &cursor, SC::ChaosBumpArena *storage) {
#ifndef NDEBUG
    int debug = cursor.tell();
    (void) debug;
#endif

    auto tag = FPropertyTag{.Name = cursor.readFName()};
    // Logger::logInfo("[Property]", "Property: {} @ {}",tag.Name.text() ,cursor.tell());
    if (tag.Name.isNone()) {
      return tag;
    }

    auto name = cursor.readFName();
    std::array<FPropertyTypeNameNode, 10> typeName;
    int32_t nodeCount = 0;
    typeName[nodeCount] = {name, cursor.read<int32_t>()};
    nodeCount++;
    int32_t remaining = typeName[0].innerCount;
    while (remaining > 0) {
      name = cursor.readFName();
      typeName[nodeCount] = {name, cursor.read<int32_t>()};
      remaining += typeName[nodeCount].innerCount - 1;
      nodeCount++;
    }

    tag.PropertyTypeName = typeName[0].Name;
    tag.PropertyType = SC::hash<EPropertyTagType>(tag.PropertyTypeName.text());
    tag.TagData = parsePropertyTagData(storage, tag.PropertyType, typeName, tag.PropertyTypeName);
    tag.Size = cursor.read<uint32_t>();
    tag.Flags = cursor.read<EPropertyTagFlags>();
    static constexpr uint8_t ExtraDataMask =
        (uint8_t) EPropertyTagFlags::HasArrayIndex |
        (uint8_t) EPropertyTagFlags::HasPropertyGuid |
        (uint8_t) EPropertyTagFlags::HasPropertyExtensions;

    if ((uint8_t) tag.Flags & ExtraDataMask) [[unlikely]] {
      if (hasFlag(tag.Flags, EPropertyTagFlags::BoolTrue))[[likely]] { tag.TagData->BoolValue = true; }
      if (hasFlag(tag.Flags, EPropertyTagFlags::HasArrayIndex)) { tag.ArrayIndex = cursor.read<uint32_t>(); }
      if (hasFlag(tag.Flags, EPropertyTagFlags::HasPropertyGuid)) { tag.PropertyGuid = cursor.read<FGuid>(); }
      if (hasFlag(tag.Flags, EPropertyTagFlags::HasPropertyExtensions)) {
        auto extensions = cursor.read<EPropertyTagExtension>();
        if (hasFlag(extensions, EPropertyTagExtension::OverridableInformation)) {
          cursor += 5;
        }
      }
    }

    return tag;
  }

  template<>
  FPropertyTagData *PP6::parsePropertyTagData(SC::ChaosBumpArena *storage, EPropertyTagType type,
                                              std::span<FPropertyTypeNameNode> typeName,
                                              const FName &name) {
    auto *data = storage->make<FPropertyTagData>();
    data->Name = name;

    switch (type) {
      case EPropertyTagType::Struct: {
        if (const auto pram = getParameter(typeName, 0); !pram.empty()) {
          data->structName = pram[0].Name;
          data->StructType = SC::hash<ESatisfactoryStruct>(data->structName.text());
        }
        break;
      }
      case EPropertyTagType::Bool: {
        data->BoolValue = false;
        break;
      }
      case EPropertyTagType::Byte:
      case EPropertyTagType::Enum: {
        if (const auto pram = getParameter(typeName, 0); !pram.empty()) {
          data->EnumName = pram[0].Name;
        }
        break;
      }
      case EPropertyTagType::Optional:
      case EPropertyTagType::Array:
      case EPropertyTagType::Set: {
        if (const auto pram = getParameter(typeName, 0); !pram.empty()) {
          data->InnerType = SC::hash<EPropertyTagType>(pram[0].Name.text());
          if (data->InnerType != EPropertyTagType::None && pram[0].innerCount != 0) {
            data->InnerTypeData = parsePropertyTagData(storage, data->InnerType, pram, pram[0].Name);
          }
        }

        break;
      }
      case EPropertyTagType::Map: {
        const auto keyType = getParameter(typeName, 0);
        const auto valType = getParameter(typeName, 1);

        if (!keyType.empty() && !valType.empty()) {
          data->InnerType = SC::hash<EPropertyTagType>(keyType[0].Name.text());
          if (data->InnerType != EPropertyTagType::None && keyType[0].innerCount != 0) {
            data->InnerTypeData = parsePropertyTagData(storage, data->InnerType, keyType, keyType[0].Name);
          }
          data->ValueType = SC::hash<EPropertyTagType>(valType[0].Name.text());
          if (data->ValueType != EPropertyTagType::None && valType[0].innerCount != 0) {
            data->ValueTypeData = parsePropertyTagData(storage, data->ValueType, valType, valType[0].Name);
          }
        }
        break;
      }
      default:
        break;
    }
    return data;
  }

  template<typename T>
  inline FPropertyValue read(ByteCursor &cursor) {
    return FPropertyValue{cursor.read<T>()};
  }

  template<>
  FPropertyValue PP6::parsePropertyValue(ByteCursor &cursor, SC::ChaosBumpArena *storage, FPropertyTag &tag,
                                         ReadType readType,
                                         uint32_t size) {
    // PropsTroughput::record(1);
    switch (tag.PropertyType) {
      case EPropertyTagType::Struct: {
        return parseStructPropertyValue(cursor, storage, tag.TagData, readType, size);
      }
      case EPropertyTagType::Array: {
        return parseArrayPropertyValue(cursor, storage, tag.TagData, readType, size);
      }
      case EPropertyTagType::Map: {
        return parseMapProperty(cursor, storage, tag.TagData, size);
      }

      case EPropertyTagType::Bool: {
        return PP3::parseBoolPropertyValue(cursor, storage, tag.TagData, readType);
      }
      case EPropertyTagType::Byte: {
        if (tag.TagData == nullptr || tag.TagData->EnumName.isNone()) {
          return FPropertyValue{cursor.read<uint8_t>()};
        }
      }
      case EPropertyTagType::Enum: {
        return FPropertyValue{cursor.readFName()};
      }
      case EPropertyTagType::Text: {
        return PP3::parseTextPropertyValue(cursor, storage);
      }
      case EPropertyTagType::Interface:
      case EPropertyTagType::Object: {
        return PP3::parseObjectPropertyValue(cursor, storage);
      }
      case EPropertyTagType::Set: {
        return PP3::parseSetPropertyValue(cursor, storage, tag.TagData);
      }
      case EPropertyTagType::Delegate: {
        cursor += sizeof(uint32_t);
      }
      case EPropertyTagType::Name: {
        return FPropertyValue{cursor.readFName()};
      }
      case EPropertyTagType::Str: {
        return FPropertyValue{cursor.readFString()};
      }

      case EPropertyTagType::Int8: {
        return FPropertyValue{cursor.read<int8_t>()};
      };
      case EPropertyTagType::Int16: {
        return FPropertyValue{cursor.read<int16_t>()};
      }
      case EPropertyTagType::Int: {
        return FPropertyValue{cursor.read<int32_t>()};
      }
      case EPropertyTagType::Int64: {
        return FPropertyValue{cursor.read<int64_t>()};
      }
      case EPropertyTagType::UInt16: {
        return FPropertyValue{cursor.read<uint16_t>()};
      }
      case EPropertyTagType::UInt32: {
        return FPropertyValue{cursor.read<uint32_t>()};
      }
      case EPropertyTagType::UInt64: {
        return FPropertyValue{cursor.read<uint64_t>()};
      }
      case EPropertyTagType::Float: {
        return FPropertyValue{cursor.read<float>()};
      }
      case EPropertyTagType::Double: {
        return FPropertyValue{cursor.read<double>()};
      }

      case EPropertyTagType::SoftObject: {
        return FPropertyValue{
          SoftObj{
            cursor.readFName(),
            cursor.readFName(),
            cursor.readArray<uint8_t>(),
          }
        };
      }

      case EPropertyTagType::FieldPath:
      case EPropertyTagType::MulticastSparse:
      case EPropertyTagType::MulticastInline: {
        cursor += size;
        break;
      }
      case EPropertyTagType::None:
        break;
      case EPropertyTagType::Unknown:
      default: {
        throw std::runtime_error("unknown property tag: " + std::string(tag.PropertyTypeName.text()) + " @ " +
                                 std::to_string(cursor.tell()));
      }
    }
    return {};
  }

  template<>
  FPropertyValue PP6::parseArrayPropertyValue(ByteCursor &cursor, SC::ChaosBumpArena *storage,
                                              FPropertyTagData *tagData,
                                              ReadType type,
                                              uint32_t size) {
    auto arrayType = tagData->InnerType;
    if (arrayType == EPropertyTagType::Unknown)[[unlikely]]
      return {};

    auto lenght = cursor.read<uint32_t>();
    FPropertyTag tag;

    std::pmr::vector<FPropertyValue> vec(cursor.m_owner->storage()->resource());
    vec.reserve(lenght);

    switch (arrayType) {
      case EPropertyTagType::Byte: {
        if ((size - sizeof(uint32_t)) > lenght) {
          tag.PropertyType = EPropertyTagType::Enum;
        } else {
          tag.PropertyType = EPropertyTagType::Byte;
        }
        for (int i = 0; i < lenght; i++) {
          vec.emplace_back(parsePropertyValue(cursor, storage, tag, type, tag.Size));
        }
        return FPropertyValue{std::move(vec)};
      }
      case EPropertyTagType::Struct: {
        tag.TagData = tagData->InnerTypeData;
      }
      default: {
        tag.PropertyType = arrayType;
      }
    }
    type = ReadType::ARRAY;
    for (int i = 0; i < lenght; i++) {
      vec.emplace_back(parsePropertyValue(cursor, storage, tag, type, tag.Size));
    }
    return FPropertyValue{std::move(vec)};
  }

  template<>
  PropertyHolder PP6::parseFallbackStruct(ByteCursor &cursor, SC::ChaosBumpArena *storage) {
    using denseMap = ankerl::unordered_dense::pmr::map<uint64_t, FPropertyTag, SC::IdentityHash>;
    auto props = cursor.m_owner->storage()->make<denseMap>(cursor.m_owner->storage()->resource());
    PropertyHolder structFallback(props);
    while (true) {
      auto tag = parsePropertyTag(cursor, storage);
      if (tag.Name.isNone()) {
        break;
      };
      auto pos = cursor.tell();
      tag.Tag = parsePropertyValue(cursor, storage, tag, ReadType::NORMAL, tag.Size);
      structFallback.m_properties->emplace(SC::hash(tag.Name.text()), std::move(tag));
      tag.TagData = nullptr;
      cursor = tag.Size + pos;
    }
    return structFallback;
  }

  template<>
  FPropertyValue PP6::parseStructPropertyValue(ByteCursor &cursor, SC::ChaosBumpArena *storage,
                                               FPropertyTagData *tagData,
                                               ReadType type,
                                               uint32_t size) {
    FPropertyValue val;
    // Logger::logInfo("[PropertyHolder]", "Struct Property:{} @ {}",  tagData->structName.text(), cursor.tell());
#ifndef NDEBUG
    auto debug = cursor.tell();
    (void) debug;
#endif
    switch (const auto structType = tagData != nullptr ? tagData->StructType : ESatisfactoryStruct::Unknown) {
      case ESatisfactoryStruct::Guid: {
        return FPropertyValue{cursor.read<FGuid>()};
      }
      case ESatisfactoryStruct::IntPoint: {
        return FPropertyValue{cursor.read<IntPoint>()};
      }
      case ESatisfactoryStruct::Vector2f:
      case ESatisfactoryStruct::DeprecateSlateVector2D: {
        return FPropertyValue{cursor.read<FVector2F>()};
      }
      case ESatisfactoryStruct::Vector4:
      case ESatisfactoryStruct::Vector4d: {
        return FPropertyValue{cursor.read<FVector4D>()};
      }
      case ESatisfactoryStruct::Vector3f: {
        return FPropertyValue{cursor.read<FVector3F>()};
      }
      case ESatisfactoryStruct::Vector2D: {
        return FPropertyValue{cursor.read<FVector2D>()};
      }
      case ESatisfactoryStruct::IntVector: {
        return FPropertyValue{cursor.read<FIntVector>()};
      }
      case ESatisfactoryStruct::Quat: {
        return FPropertyValue{cursor.read<FQuat>()};
      }
      case ESatisfactoryStruct::Box3f: {
        return FPropertyValue{cursor.read<FBox3f>()};
      }
      case ESatisfactoryStruct::Box2f: {
        return FPropertyValue{cursor.read<FBox2f>()};
      }
      case ESatisfactoryStruct::Box: {
        return FPropertyValue{cursor.read<FBox>()};
      }
      case ESatisfactoryStruct::Vector: {
        return FPropertyValue{cursor.read<FVector>()};
      }
      case ESatisfactoryStruct::LinearColor: {
        return FPropertyValue{cursor.read<FLinearColor>()};
      }
      case ESatisfactoryStruct::Color: {
        return FPropertyValue{cursor.read<FColor>()};
      }
      case ESatisfactoryStruct::RichCurveKey: {
        return FPropertyValue{cursor.read<FRichCurveKey>()};
      }

      case ESatisfactoryStruct::Rotator: {
        return FPropertyValue{cursor.read<FRotator>()};
      }

      case ESatisfactoryStruct::IntVector2: {
        return FPropertyValue{cursor.read<FIntVector2>()};
      }
      // --- Sequencer / MovieScene (Cutscene Data) ---
      case ESatisfactoryStruct::MovieSceneTrackIdentifier:
        cursor += 4;
        break;
      case ESatisfactoryStruct::MovieSceneTrackIdentifiers:
      case ESatisfactoryStruct::MovieSceneFloatChannel:
      case ESatisfactoryStruct::MovieSceneDoubleChannel:
      case ESatisfactoryStruct::MovieSceneFrameRange:
      case ESatisfactoryStruct::MovieSceneEvaluationFieldEntityTree:
      case ESatisfactoryStruct::MovieSceneSequenceID:
      case ESatisfactoryStruct::MovieSceneEvaluationKey:
      case ESatisfactoryStruct::MovieSceneEventParameters:
      case ESatisfactoryStruct::MovieSceneSegment:
      case ESatisfactoryStruct::MovieSceneSegmentIdentifier:
      case ESatisfactoryStruct::MovieSceneTrackFieldData:
      case ESatisfactoryStruct::MovieSceneSubSequenceTree:
      case ESatisfactoryStruct::MovieSceneSequenceInstanceDataPtr:
      case ESatisfactoryStruct::MovieSceneEvalTemplatePtr:
      case ESatisfactoryStruct::MovieSceneTrackImplementationPtr:
      case ESatisfactoryStruct::MovieSceneByteChannel:
      case ESatisfactoryStruct::SectionEvaluationDataTree:

      // --- Niagara / VFX (Particle-Logic) ---
      case ESatisfactoryStruct::NiagaraUserRedirectionParameterStore:
      case ESatisfactoryStruct::NiagaraVariable:
      case ESatisfactoryStruct::NiagaraVariableBase:
      case ESatisfactoryStruct::NiagaraVariableWithOffset:
      case ESatisfactoryStruct::NiagaraDataChannelVariable:
      case ESatisfactoryStruct::NiagaraDataInterfaceGPUParamInfo:
      case ESatisfactoryStruct::NiagaraScript:
      case ESatisfactoryStruct::NiagaraScriptExecutionParameterStore:

      // --- UI / Fonts / Special-System ---
      case ESatisfactoryStruct::FontData:
      case ESatisfactoryStruct::FontCharacter:
      case ESatisfactoryStruct::LevelSequenceObjectReferenceMap:
      case ESatisfactoryStruct::SmartName:

      // Data Bloat
      case ESatisfactoryStruct::InstancedStruct:
      case ESatisfactoryStruct::Transform:
      case ESatisfactoryStruct::DateTime:
      case ESatisfactoryStruct::FrameNumber:
      case ESatisfactoryStruct::FGServerCertificate:
      case ESatisfactoryStruct::PerPlatformFrameRate:
      case ESatisfactoryStruct::ShaderValueTypeHandle:
      case ESatisfactoryStruct::GameplayTagContainer:
      case ESatisfactoryStruct::InventoryItem: {
        cursor += size;
        break;
      }

      case ESatisfactoryStruct::PerQualityLevelInt: {
        return FPropertyValue{FPerQualityLevelInt::get(cursor)};
        break;
      }
      case ESatisfactoryStruct::PerQualityLevelFloat: {
        return FPropertyValue{FPerQualityLevelFloat::get(cursor)};
        break;
      }
      case ESatisfactoryStruct::CompressedRichCurve: {
        return FPropertyValue{FCompressedRichCurve::get(cursor)};
      }

      case ESatisfactoryStruct::MaterialOverrideNanite: {
        return FPropertyValue{
          FMaterialOverrideNanite{
            cursor.readFName(), cursor.readBool(),
            cursor.m_owner->resolveIndex(cursor.read<int32_t>())

          }
        };
      }
      case ESatisfactoryStruct::SkeletalMeshSamplingLODBuiltData: {
        return FPropertyValue{
          FWeightedRandomSampler{
            .probabilities = cursor.readArray<float>(),
            .alias = cursor.readArray<int32_t>(),
            .totlalWeight = cursor.read<float>(),
          }
        };
      }
      case ESatisfactoryStruct::SoftObjectPath:
      case ESatisfactoryStruct::SoftClassPath: {
        return FPropertyValue{
          SoftObj{
            cursor.readFName(),
            cursor.readFName(),
            cursor.readArray<uint8_t>()
          }
        };
      }
      case ESatisfactoryStruct::PerPlatformInt: {
        cursor.readBool();
        return FPropertyValue{cursor.read<int32_t>()};
      }
      case ESatisfactoryStruct::PerPlatformFloat: {
        cursor.readBool();
        return FPropertyValue{cursor.read<float>()};
      }

      case ESatisfactoryStruct::KeyHandleMap:
        return FPropertyValue{PropertyHolder{}};

      case ESatisfactoryStruct::UniversalObjectLocatorFragment: {
        auto FragmentTypeID = cursor.readFName();
      }
      default:
        // #ifndef NDEBUG
        //         if (tagData) {
        //           propertyLOG::info("Parsing Struct: {} as Fallback @ {}", tagData->structName.text(),
        //                           cursor.tell());
        //         }
        // #endif
        return FPropertyValue{parseFallbackStruct(cursor, storage)};
    }
    return val;
  }

  template<>
  FPropertyValue PP6::parseMapProperty(ByteCursor &cursor, SC::ChaosBumpArena *storage, FPropertyTagData *tagData,
                                       uint32_t size) {
    if (!tagData->InnerTypeData && !tagData->ValueTypeData) {
      cursor += size;
      return {};
    }
    FPropertyTag keyTag{.PropertyType = tagData->InnerType, .TagData = tagData->InnerTypeData};
    FPropertyTag valueTag{.PropertyType = tagData->ValueType, .TagData = tagData->ValueTypeData};

    auto keysToRemove = cursor.read<int32_t>();
    for (auto i = 0; i < keysToRemove; i++) {
      parsePropertyValue(cursor, storage, keyTag, ReadType::MAP, 0);
    }

    std::pmr::vector<std::pair<FPropertyValue, FPropertyValue> > vec(cursor.m_owner->storage()->resource());
    auto entries = cursor.read<uint32_t>();
    vec.reserve(entries);
    for (uint32_t i = 0; i < entries; i++) {
      vec.emplace_back(parsePropertyValue(cursor, storage, keyTag, ReadType::MAP, 0),
                       parsePropertyValue(cursor, storage, valueTag, ReadType::MAP, 0));
    }

    return {.data = std::move(vec)};
  }
}
