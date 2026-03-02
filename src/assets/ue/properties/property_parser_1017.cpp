//
// Created by oleub on 23.02.26.
//

#include "AssetParser.hpp"
#include "property_parser.hpp"
#include "thread_arena.hpp"
#include "core/EClassSerializationControlExtension.hpp"


namespace SAA {
  template<>
  void PP6::parse(PropertyHolder *holder, ByteCursor &cursor) {
    auto controlByte = cursor.read<EClassSerializationControlExtension>();
    if (hasFlag(controlByte, EClassSerializationControlExtension::OverridableSerializationInformation)) {
      auto op = cursor.read<uint8_t>();
    }

    while (true) {
      auto tag = parsePropertyTag(cursor);
      if (tag.Name.isNone()) {
        return;
      }
      auto final = cursor.tell() + tag.Size;
      tag.Tag = parsePropertyValue(cursor, tag, ReadType::NORMAL, tag.Size);
      cursor = final;
      holder->m_properties.emplace(fnv1a(tag.Name.text()), std::move(tag));
    }
  }

  template<>
  FPropertyTag PP6::parsePropertyTag(ByteCursor &cursor) {
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
    auto *firstNode = ThreadArena::make<FPropertyTypeNameNode>(
      name, cursor.read<int32_t>());

    int32_t nodeCount = 1;
    int32_t remaining = firstNode->innerCount;
    while (remaining > 0) {
      name = cursor.readFName();
      const auto *node = ThreadArena::make<FPropertyTypeNameNode>(name, cursor.read<int32_t>());
      remaining += node->innerCount - 1;
      nodeCount++;
    }
    /**
 * THE SPAN TRICK:
 * Exploits the strict linear allocation of the ThreadLocal Arena.
 * Since 'ThreadArena::make' guarantees contiguous memory for sequential calls
 * on the same thread, we can treat individually allocated nodes as a packed array.
 * This avoids 'std::vector' overhead/reallocations while parsing recursive
 * trees into a cache-friendly, flat 'std::span'.
 */
    auto typeName = std::span(firstNode, nodeCount);

    tag.PropertyTypeName = typeName[0].Name;
    tag.PropertyType = hashEnum<EPropertyTagType>(tag.PropertyTypeName.text());
    tag.TagData = parsePropertyTagData(tag.PropertyType, typeName, tag.PropertyTypeName);
    tag.Size = cursor.read<uint32_t>();
    tag.Flags = cursor.read<EPropertyTagFlags>();
    if (hasFlag(tag.Flags, EPropertyTagFlags::BoolTrue)) { tag.TagData->BoolValue = true; }
    if (hasFlag(tag.Flags, EPropertyTagFlags::HasArrayIndex)) { tag.ArrayIndex = cursor.read<uint32_t>(); }
    if (hasFlag(tag.Flags, EPropertyTagFlags::HasPropertyGuid)) { tag.PropertyGuid = cursor.read<FGuid>(); }
    if (hasFlag(tag.Flags, EPropertyTagFlags::HasPropertyExtensions)) {
      auto extensions = cursor.read<EPropertyTagExtension>();
      if (hasFlag(extensions, EPropertyTagExtension::OverridableInformation)) {
        cursor += 5;
      }
    }

    return tag;
  }

  template<>
  FPropertyTagData *PP6::parsePropertyTagData(EPropertyTagType type, std::span<FPropertyTypeNameNode> typeName,
                                              const FName &name) {
    auto *data = ThreadArena::make<FPropertyTagData>();
    data->Name = name;

    switch (type) {
      case EPropertyTagType::Struct: {
        if (const auto pram = getParameter(typeName, 0); !pram.empty()) {
          data->structName = pram[0].Name;
          data->StructType = hashEnum<ESatisfactoryStruct>(data->structName.text());
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
          data->InnerType = hashEnum<EPropertyTagType>(pram[0].Name.text());
          if (data->InnerType != EPropertyTagType::None && pram[0].innerCount != 0) {
            data->InnerTypeData = parsePropertyTagData(data->InnerType, pram, pram[0].Name);
          }
        }

        break;
      }
      case EPropertyTagType::Map: {
        const auto keyType = getParameter(typeName, 0);
        const auto valType = getParameter(typeName, 1);

        if (!keyType.empty() && !valType.empty()) {
          data->InnerType = hashEnum<EPropertyTagType>(keyType[0].Name.text());
          if (data->InnerType != EPropertyTagType::None && keyType[0].innerCount != 0) {
            data->InnerTypeData = parsePropertyTagData(data->InnerType, keyType, keyType[0].Name);
          }
          data->ValueType = hashEnum<EPropertyTagType>(valType[0].Name.text());
          if (data->ValueType != EPropertyTagType::None && valType[0].innerCount != 0) {
            data->ValueTypeData = parsePropertyTagData(data->ValueType, valType, valType[0].Name);
          }
        }
        break;
      }
      default:
        break;
    }
    return data;
  }

  template<>
  FPropertyValue PP6::parsePropertyValue(ByteCursor &cursor, FPropertyTag &tag, ReadType readType,
                                         uint32_t size) {
    FPropertyValue val;
    switch (tag.PropertyType) {
      case EPropertyTagType::Struct: {
        val = parseStructPropertyValue(cursor, tag.TagData, readType, size);
        break;
      }
      case EPropertyTagType::Array: {
        val = parseArrayPropertyValue(cursor, tag.TagData, readType, size);
        break;
      }
      case EPropertyTagType::Map: {
        val = parseMapProperty(cursor, tag.TagData, size);
        break;
      }

      case EPropertyTagType::Bool: {
        val = PP3::parseBoolPropertyValue(cursor, tag.TagData, readType);
        break;
      }
      case EPropertyTagType::Byte: {
        if (tag.TagData == nullptr || tag.TagData->EnumName.isNone()) {
          val.data = cursor.read<uint8_t>();
          break;
        }
      }
      case EPropertyTagType::Enum: {
        val.data = cursor.readFName();
        break;
      }
      case EPropertyTagType::Text: {
        val = PP3::parseTextPropertyValue(cursor);
        break;
      }
      case EPropertyTagType::Interface:
      case EPropertyTagType::Object: {
        val = PP3::parseObjectPropertyValue(cursor);
        break;
      }
      case EPropertyTagType::Set: {
        val = PP3::parseSetPropertyValue(cursor, tag.TagData);
        break;
      }
      case EPropertyTagType::Delegate: {
        auto num = cursor.read<int32_t>();
        val.data = cursor.readFName();
        break;
      }
      case EPropertyTagType::Name: {
        val.data = cursor.readFName();
        break;
      }
      case EPropertyTagType::Str: {
        val.data = cursor.readFString();
        break;
      }

      case EPropertyTagType::Int8: {
        val.data = cursor.read<int8_t>();
        break;
      };
      case EPropertyTagType::Int16: {
        val.data = cursor.read<int16_t>();
        return val;
      }
      case EPropertyTagType::Int: {
        val.data = cursor.read<int32_t>();
        return val;
      }
      case EPropertyTagType::Int64: {
        val.data = cursor.read<int64_t>();
        return val;
      }
      case EPropertyTagType::UInt16: {
        val.data = cursor.read<uint16_t>();
        return val;
      }
      case EPropertyTagType::UInt32: {
        val.data = cursor.read<uint32_t>();
        return val;
      }
      case EPropertyTagType::UInt64: {
        val.data = cursor.read<uint64_t>();
        return val;
      }
      case EPropertyTagType::Float: {
        val.data = cursor.read<float>();
        return val;
      }
      case EPropertyTagType::Double: {
        val.data = cursor.read<double>();
        return val;
      }

      case EPropertyTagType::SoftObject: {
       val.data = SoftObj{
          cursor.readFName(),
          cursor.readFName(),
          cursor.readArray<uint8_t>(),
        };
        break;
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
      default:

        throw std::runtime_error("unknown property tag: " + std::string(tag.PropertyTypeName.text()) + " @ " +
                                 std::to_string(cursor.tell()));
    }
    return val;
  }

  template<>
  FPropertyValue PP6::parseArrayPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                              uint32_t size) {
    FPropertyValue val;
    auto arrayType = tagData->InnerType;
    if (arrayType == EPropertyTagType::Unknown)
      return val;

    auto lenght = cursor.read<uint32_t>();
    FPropertyTag tag;
    if (arrayType == EPropertyTagType::Struct) {
      tag.TagData = tagData->InnerTypeData;
    }

    std::pmr::vector<FPropertyValue> vec(cursor.m_owner->storage()->memoryResource());
    vec.reserve(lenght);

    switch (arrayType) {
      case EPropertyTagType::Byte: {
        if (size == sizeof(uint32_t) ? false : (size - sizeof(uint32_t)) / lenght > 1) {
          tag.PropertyType = EPropertyTagType::Enum;
        } else {
          tag.PropertyType = EPropertyTagType::Byte;
        }
        for (int i = 0; i < lenght; i++) {
          vec.emplace_back(parsePropertyValue(cursor, tag, type, tag.Size));
        }
        val.data = std::move(vec);
        return val;
      }
      default: {
        tag.PropertyType = arrayType;
      }
    }
    type = ReadType::ARRAY;
    for (int i = 0; i < lenght; i++) {
      vec.emplace_back(parsePropertyValue(cursor, tag, type, tag.Size));
    }
    val.data = std::move(vec);

    return val;
  }

  template<>
  PropertyHolder PP6::parseFallbackStruct(ByteCursor &cursor) {
    PropertyHolder structFallback(cursor.m_owner->storage()->memoryResource());
    while (true) {
      auto tag = parsePropertyTag(cursor);
      if (tag.Name.isNone()) {
        break;
      };
      auto pos = cursor.tell();
      tag.Tag = parsePropertyValue(cursor, tag, ReadType::NORMAL, tag.Size);
      structFallback.m_properties.emplace(fnv1a(tag.Name.text()), std::move(tag));
      tag.TagData = nullptr;
      cursor = tag.Size + pos;
    }
    return structFallback;
  }

  template<>
  FPropertyValue PP6::parseStructPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type,
                                               uint32_t size) {
    FPropertyValue val;
    // Logger::logInfo("[PropertyHolder]", "Struct Property:{} @ {}",  tagData->structName.text(), cursor.tell());
#ifndef NDEBUG
    auto debug = cursor.tell();
    (void) debug;
#endif
    switch (const auto structType = tagData != nullptr ? tagData->StructType : ESatisfactoryStruct::Unknown) {
      case ESatisfactoryStruct::Guid: {
        val.data = cursor.read<FGuid>();
        break;
      }
      case ESatisfactoryStruct::IntPoint: {
        val.data = cursor.read<IntPoint>();
        break;
      }
      case ESatisfactoryStruct::Vector2f:
      case ESatisfactoryStruct::DeprecateSlateVector2D: {
        val.data = cursor.read<FVector2F>();
        break;
      }
      case ESatisfactoryStruct::Vector4:
      case ESatisfactoryStruct::Vector4d: {
        val.data = cursor.read<FVector4d>();
        break;
      }
      case ESatisfactoryStruct::Vector3f: {
        val.data = cursor.read<FVector_f>();
        break;
      }
      case ESatisfactoryStruct::Vector2D: {
        val.data = cursor.read<FVector2D>();
        break;
      }
      case ESatisfactoryStruct::IntVector: {
        val.data = cursor.read<FIntVector>();
        break;
      }
      case ESatisfactoryStruct::Quat: {
        val.data = cursor.read<FQuat>();
        break;
      }
      case ESatisfactoryStruct::Box2f: {
        val.data = cursor.read<FBox2f>();
        break;
      }
      case ESatisfactoryStruct::Box: {
        val.data = cursor.read<FBox>();
        break;
      }
      case ESatisfactoryStruct::Vector: {
        val.data = cursor.read<FVector>();
        break;
      }
      case ESatisfactoryStruct::LinearColor: {
        val.data = cursor.read<FLinearColor>();
        break;
      }
      case ESatisfactoryStruct::Color: {
        val.data = cursor.read<FColor>();
        break;
      }
      case ESatisfactoryStruct::RichCurveKey: {
        val.data = cursor.read<FRichCurveKey>();
        break;
      }

      case ESatisfactoryStruct::Rotator: {
        val.data = cursor.read<FRotator>();
        break;
      }
      // --- Sequencer / MovieScene (Cutscene Data) ---
      case ESatisfactoryStruct::MovieSceneTrackIdentifier:
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
      case ESatisfactoryStruct::DateTime:
      case ESatisfactoryStruct::FrameNumber:
      case ESatisfactoryStruct::FGServerCertificate:
      case ESatisfactoryStruct::PerPlatformFrameRate:
      case ESatisfactoryStruct::ShaderValueTypeHandle:
      case ESatisfactoryStruct::InventoryItem: {
        cursor += size;
        break;
      }

      case ESatisfactoryStruct::PerQualityLevelInt: {
        val.data = FPerQualityLevelInt::get(cursor);
        break;
      }
      case ESatisfactoryStruct::PerQualityLevelFloat: {
        val.data = FPerQualityLevelFloat::get(cursor);
        break;
      }
      case ESatisfactoryStruct::CompressedRichCurve: {
        val.data = FCompressedRichCurve::get(cursor);
        break;
      }

      case ESatisfactoryStruct::MaterialOverrideNanite: {
        val.data = FMaterialOverrideNanite{
          cursor.readFName(), cursor.readBool(),
          cursor.m_owner->resolveIndex(cursor.read<int32_t>())

        };
        break;
      }
      case ESatisfactoryStruct::SkeletalMeshSamplingLODBuiltData: {
        val.data = FWeightedRandomSampler{
          .probabilities = cursor.readArray<float>(),
          .alias = cursor.readArray<int32_t>(),
          .totlalWeight = cursor.read<float>(),
        };
        break;
      }
      case ESatisfactoryStruct::SoftObjectPath:
      case ESatisfactoryStruct::SoftClassPath: {
        val.data = SoftObj{
          cursor.readFName(),
          cursor.readFName(),
          cursor.readArray<uint8_t>()
        };
        break;
      }
      case ESatisfactoryStruct::PerPlatformInt: {
        cursor.readBool();
        val.data = cursor.read<int32_t>();
        break;
      }
      case ESatisfactoryStruct::PerPlatformFloat: {
        cursor.readBool();
        val.data = cursor.read<float>();
        break;
      }

      case ESatisfactoryStruct::KeyHandleMap:
        val.data = PropertyHolder{};
        break;
      default:
        // #ifndef NDEBUG
        //         if (tagData) {
        //           Logger::logInfo("[PropertyHolder]", "Parsing Struct: {} as Fallback @ {}", tagData->structName.text(),
        //                           cursor.tell());
        //         }
        // #endif
        val.data = parseFallbackStruct(cursor);
        break;
    }
    return val;
  }

  template<>
  FPropertyValue PP6::parseMapProperty(ByteCursor &cursor, FPropertyTagData *tagData, uint32_t size) {
    if (!tagData->InnerTypeData && !tagData->ValueTypeData) {
      cursor += size;
      return {};
    }
    FPropertyTag keyTag{.PropertyType = tagData->InnerType, .TagData = tagData->InnerTypeData};
    FPropertyTag valueTag{.PropertyType = tagData->ValueType, .TagData = tagData->ValueTypeData};

    auto keysToRemove = cursor.read<int32_t>();
    for (auto i = 0; i < keysToRemove; i++) {
      parsePropertyValue(cursor, keyTag, ReadType::MAP, 0);
    }

    std::pmr::vector<std::pair<FPropertyValue, FPropertyValue> > vec(cursor.m_owner->storage()->memoryResource());
    auto entries = cursor.read<uint32_t>();
    vec.reserve(entries);
    for (uint32_t i = 0; i < entries; i++) {
      vec.emplace_back(parsePropertyValue(cursor, keyTag, ReadType::MAP, 0),
                       parsePropertyValue(cursor, valueTag, ReadType::MAP, 0));
    }

    return {.data = std::move(vec)};
  }
}
