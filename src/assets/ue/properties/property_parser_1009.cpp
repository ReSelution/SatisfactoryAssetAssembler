//
// Created by oleub on 23.02.26.
//

#include <algorithm>
#include <format>
#include <string>

#include "f_property_tag.hpp"
#include "property_parser.hpp"
#include "sc/localizations/magpie_string.hpp"

namespace SAA {
  template<>
  FPropertyTag PropertyParser<EUE5Version::UE5_3>::parsePropertyTag(ByteCursor &cursor, sc::MonotonicResource *storage) {
    // Logger::logInfo("[Property]", "Property @ {}", cursor.tell());
    int debug = cursor.tell();
    (void) debug;
    auto tag = FPropertyTag{.Name = cursor.readFName()};
    // Logger::logInfo("[Property]", "Property: {} @ {}",tag.Name.text() ,cursor.tell());

    if (tag.Name.isNone()) {
      return tag;
    }


    tag.PropertyTypeName = cursor.readFName();
    // Logger::logInfo("[Property]", "Property {} @ {}", tag.PropertyTypeName.text(), cursor.tell());
    tag.PropertyType = sc::hash<EPropertyTagType>(tag.PropertyTypeName.text());
    tag.Size = cursor.read<uint32_t>();
    tag.ArrayIndex = cursor.read<uint32_t>();
    tag.TagData = parsePropertyTagData(cursor, storage, tag.PropertyType);
    if (cursor.readFlag()) {
      tag.PropertyGuid = cursor.read<FGuid>();
    }
    return tag;
  }

  template<>
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parseMapProperty(ByteCursor &cursor, sc::MonotonicResource *storage,
                                                                      FPropertyTagData *tagData, uint32_t size) {
    // UE 5.3 does not store which struct a key or value is, such a map can only be skipped. Its size is always the
    // one of its tag: UE has no nested containers, a map is never an element of an array, set or another map.
    if (tagData->InnerType == EPropertyTagType::Struct || tagData->ValueType == EPropertyTagType::Struct) {
      if (size == 0) [[unlikely]] {
        throw std::runtime_error("Map of structs without a size cannot be skipped @ " + std::to_string(cursor.tell()));
      }
      cursor += size;
      return {};
    }
    const auto start = cursor.tell();
    FPropertyTag keyTag{.PropertyType = tagData->InnerType, .TagData = tagData->InnerTypeData};
    FPropertyTag valueTag{.PropertyType = tagData->ValueType, .TagData = tagData->ValueTypeData};

    auto keysToRemove = cursor.read<int32_t>();
    for (auto i = 0; i < keysToRemove; i++) {
      parsePropertyValue(cursor, storage, keyTag, ReadType::MAP, 0);
    }

    std::pmr::vector<std::pair<FPropertyValue, FPropertyValue> > vec(cursor.m_owner->storage());
    auto entries = cursor.read<uint32_t>();
    // Every entry takes at least two bytes, anything above that is a broken count.
    if (entries > cursor.remaining() / 2) [[unlikely]] {
      throw std::runtime_error("Map entry count exceeds the remaining data @ " + std::to_string(cursor.tell()));
    }
    vec.reserve(entries);
    for (uint32_t i = 0; i < entries; i++) {
      vec.emplace_back(parsePropertyValue(cursor, storage, keyTag, ReadType::MAP, 0),
                       parsePropertyValue(cursor, storage, valueTag, ReadType::MAP, 0));
    }

    // A ByteProperty key or value is either one byte or an enum stored as a name, and the tag of a map does not say
    // which. If the map does not end where its size says, the guess was wrong and the content is dropped.
    if (size != 0 && cursor.tell() != start + size) [[unlikely]] {
      cursor = start + size;
      return {};
    }
    return {.data = std::move(vec)};
  }

  template<>
  FPropertyTagData *PropertyParser<
    EUE5Version::UE5_3>::parsePropertyTagData(ByteCursor &cursor, sc::MonotonicResource *storage, EPropertyTagType type) {
    auto *data = storage->make<FPropertyTagData>();
    switch (type) {
      case EPropertyTagType::Struct: {
        data->structName = cursor.readFName();
        data->StructType = sc::hash<ESatisfactoryStruct>(data->structName.text());
        data->StructGuid = cursor.read<FGuid>();
        break;
      }
      case EPropertyTagType::Bool: {
        data->BoolValue = cursor.readFlag();
        break;
      }
      case EPropertyTagType::Byte:
      case EPropertyTagType::Enum: {
        data->EnumName = cursor.readFName();
        break;
      }
      case EPropertyTagType::Array:
      case EPropertyTagType::Set: {
        data->InnerType = sc::hash<EPropertyTagType>(cursor.readFName().text());
        break;
      }
      case EPropertyTagType::Map: {
        data->InnerType = sc::hash<EPropertyTagType>(cursor.readFName().text());
        data->ValueType = sc::hash<EPropertyTagType>(cursor.readFName().text());
        break;
      }
      case EPropertyTagType::Optional: {
        data->InnerType = sc::hash<EPropertyTagType>(cursor.readFName().text());
        break;
      }
      default:
        break;
    }
    return data;
  }

  template<>
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parsePropertyValue(ByteCursor &cursor, sc::MonotonicResource *storage,
                                                                        FPropertyTag &tag,
                                                                        ReadType readType,
                                                                        uint32_t size) {
    FPropertyValue val;
    switch (tag.PropertyType) {
      case EPropertyTagType::Struct: {
        val = parseStructPropertyValue(cursor, storage, tag.TagData, readType, size);
        break;
      }
      case EPropertyTagType::Array: {
        val = parseArrayPropertyValue(cursor, storage, tag.TagData, readType, size);
        break;
      }
      case EPropertyTagType::Map: {
        val = parseMapProperty(cursor, storage, tag.TagData, size);
        break;
      }

      case EPropertyTagType::Bool: {
        val = parseBoolPropertyValue(cursor, storage, tag.TagData, readType);
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
        val = parseTextPropertyValue(cursor, storage);
        break;
      }
      case EPropertyTagType::Interface:
      case EPropertyTagType::Object: {
        val = parseObjectPropertyValue(cursor, storage);
        break;
      }
      case EPropertyTagType::Set: {
        val = parseSetPropertyValue(cursor, storage, tag.TagData);
        break;
      }
      case EPropertyTagType::Delegate: {
        (void) cursor.read<int32_t>(); // num
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
        // Same as the skipped structs in parseStructPropertyValue(): in a map or a set there is no size.
        if (size == 0) [[unlikely]] {
          throw std::runtime_error("Field path or multicast delegate without a size cannot be skipped @ " +
                                   std::to_string(cursor.tell()));
        }
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
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parseStructPropertyValue(
    ByteCursor &cursor, sc::MonotonicResource *storage, FPropertyTagData *tagData, ReadType type,
    uint32_t size) {
    FPropertyValue val;
    // Logger::logInfo("[PropertyHolder]", "Struct Property:{} @ {}",  tagData->structName.text(), cursor.tell());
    auto debug = cursor.tell();
    (void) debug;
    switch (tagData != nullptr ? tagData->StructType : ESatisfactoryStruct::Unknown) {
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
        val.data = cursor.read<FVector4D>();
        break;
      }
      case ESatisfactoryStruct::Vector3f: {
        val.data = cursor.read<FVector3F>();
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
        // Without a size skipping does nothing and everything after it would be read from the wrong position, the
        // file fails instead. Does not occur in 1009.
        if (size == 0) [[unlikely]] {
          throw std::runtime_error("Struct " + std::string(tagData->structName.text()) +
                                   " without a size cannot be skipped @ " + std::to_string(cursor.tell()));
        }
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
        //         if (tagData) {
        //           Logger::logInfo("[PropertyHolder]", "Parsing Struct: {} as Fallback @ {}", tagData->structName.text(),
        //                           cursor.tell());
        //         }
        val.data = PropertyParser<EUE5Version::UE5_3>::parseFallbackStruct(cursor, storage);
        break;
    }
    return val;
  }

  template<>
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parseTextPropertyValue(
    ByteCursor &cursor, sc::MonotonicResource *storage) {
    FText text{.flags = cursor.read<ETextFlag>(), .type = cursor.read<ETextHistoryType>()};

    sc::MagpieString magpieResult;
    switch (text.type) {
      case ETextHistoryType::Base: {
        text.nSpace = cursor.readFString();
        text.key = cursor.readFString();
        text.sourceString = cursor.readFString();
        magpieResult = sc::MagpieString(text.nSpace, text.key, text.sourceString);
        break;
      }
      case ETextHistoryType::AsPercent: {
        // FTextHistory_FormatNumber: the source value, optional formatting options and the target culture.
        double value = 0;
        switch (cursor.read<EFormatArgumentType>()) {
          case EFormatArgumentType::Int:
            value = static_cast<double>(cursor.read<int64_t>());
            break;
          case EFormatArgumentType::UInt:
            value = static_cast<double>(cursor.read<uint64_t>());
            break;
          case EFormatArgumentType::Float:
            value = cursor.read<float>();
            break;
          case EFormatArgumentType::Double:
            value = cursor.read<double>();
            break;
          default:
            throw std::runtime_error("AsPercent text with a value that is not a number @ " +
                                     std::to_string(cursor.tell()));
        }
        // Without options the culture decides, and that is no fractional digits for a percentage.
        int32_t minFraction = 0;
        int32_t maxFraction = 0;
        if (cursor.readBool()) {
          // Field by field: the struct is stored without padding (25 bytes), in memory it has 28.
          cursor += 2 * sizeof(uint32_t) + sizeof(ERoundingMode) + 2 * sizeof(int32_t);
          minFraction = cursor.read<int32_t>();
          maxFraction = cursor.read<int32_t>();
        }
        (void) cursor.readFString(); // targetCulture

        // Digit grouping, the sign option and the rounding mode are not applied, neither is the percent pattern of
        // the culture ("100 %" in German).
        std::string percent = std::format("{:.{}f}", value * 100.0, std::clamp(maxFraction, 0, 15));
        if (const auto dot = percent.find('.'); dot != std::string::npos) {
          const size_t keep = dot + 1 + static_cast<size_t>(std::clamp(minFraction, 0, 15));
          while (percent.size() > keep && percent.back() == '0') {
            percent.pop_back();
          }
          if (percent.back() == '.') {
            percent.pop_back();
          }
        }
        percent += '%';
        magpieResult = sc::MagpieString::fromStr(percent);
        break;
      }
      case ETextHistoryType::Transform: {
        auto innerValue = parseTextPropertyValue(cursor, storage);
        magpieResult = std::get<sc::MagpieString>(innerValue.data);
        cursor.read<ETransformType>();
        break;
      }

      case ETextHistoryType::NamedFormat:
      case ETextHistoryType::OrderedFormat:
      case ETextHistoryType::ArgumentFormat:
      case ETextHistoryType::AsNumber:
      case ETextHistoryType::AsCurrency:
      case ETextHistoryType::AsDate:
      case ETextHistoryType::AsTime:
      case ETextHistoryType::AsDateTime:
      case ETextHistoryType::TextGenerator:
        throw std::runtime_error("Not implemented");
      case ETextHistoryType::StringTableEntry: {
        const auto tableId = cursor.readFName();
        const auto key = cursor.readFString();
        magpieResult = sc::MagpieString(tableId.text(), key);
        break;
      }
      default:
        if (cursor.readBool()) {
          text.textHistory.text = cursor.readFString();
          magpieResult = sc::MagpieString::fromStr(text.textHistory.text);
        }
    }

    return {.data = magpieResult};
  }

  template<>
  PropertyHolder PropertyParser<EUE5Version::UE5_3>::parseFallbackStruct(
    ByteCursor &cursor, sc::MonotonicResource *storage) {
    PropertyMapBuilder properties;
    while (true) {
      auto tag = parsePropertyTag(cursor, storage);
      if (tag.Name.isNone()) {
        break;
      };
      const auto end = cursor.tell() + tag.Size;
      tag.Tag = parsePropertyValue(cursor, storage, tag, ReadType::NORMAL, tag.Size);
      properties.add(PropertyHolder::propertyKey(sc::hash(tag.Name.text()), tag.ArrayIndex), std::move(tag));
      cursor = end;
    }
    return PropertyHolder(properties.build(cursor.m_owner->storage()));
  }

  template<>
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parseArrayPropertyValue(
    ByteCursor &cursor, sc::MonotonicResource *storage, FPropertyTagData *tagData, ReadType type,
    uint32_t size) {
    FPropertyValue val;
    auto arrayType = tagData->InnerType;
    if (arrayType == EPropertyTagType::Unknown)
      return val;

    auto lenght = cursor.read<uint32_t>();
    // Every element takes at least one byte, anything above that is a broken length.
    if (lenght > cursor.remaining()) [[unlikely]] {
      throw std::runtime_error("Array length exceeds the remaining data @ " + std::to_string(cursor.tell()));
    }
    FPropertyTag tag;
    // if (arrayType == EPropertyTagType::Struct) {
    //   tag = parsePropertyTag(cursor);
    // }

    // Plain bytes stay where they are in the file buffer. As elements of the vector below every byte would take
    // sizeof(FPropertyValue), a 16 MiB array then needs 1.9 GB.
    // An enum stored as a name takes more than one byte per element. Only looked at for byte arrays: other arrays
    // can be empty with a size above 4, which would divide by zero here.
    const bool isEnumArray = arrayType == EPropertyTagType::Byte && lenght != 0 &&
                             (size - sizeof(uint32_t)) / lenght > 1;
    if (arrayType == EPropertyTagType::Byte && !isEnumArray) {
      val.data = cursor.readArray<uint8_t>(lenght);
      return val;
    }

    std::pmr::vector<FPropertyValue> vec(cursor.m_owner->storage());
    vec.reserve(lenght);

    switch (arrayType) {
      case EPropertyTagType::Byte: {
        tag.PropertyType = EPropertyTagType::Enum;
        for (int i = 0; i < lenght; i++) {
          vec.emplace_back(parsePropertyValue(cursor, storage, tag, type, tag.Size));
        }
        val.data = std::move(vec);
        return val;
      }
      case EPropertyTagType::Struct: {
        tag = parsePropertyTag(cursor, storage);
      }
      default: {
        tag.PropertyType = arrayType;
      }
    }
    type = ReadType::ARRAY;
    for (int i = 0; i < lenght; i++) {
      vec.emplace_back(parsePropertyValue(cursor, storage, tag, type, tag.Size / lenght));
    }
    val.data = std::move(vec);

    return val;
  }

  template<>
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parseObjectPropertyValue(
    ByteCursor &cursor, sc::MonotonicResource *storage) {
    FPropertyValue val;
    auto index = cursor.read<int32_t>();
    val.data = cursor.m_owner->resolveIndex(index);
    return val;
  }

  template<>
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parseSetPropertyValue(
    ByteCursor &cursor, sc::MonotonicResource *storage, FPropertyTagData *tagData) {
    auto num = cursor.read<uint32_t>();
    FPropertyTag tag{
      .PropertyType = tagData->InnerType,
      .TagData = tagData->InnerTypeData,
    };

    for (int i = 0; i < num; i++) {
      parsePropertyValue(cursor, storage, tag, ReadType::ARRAY, 0);
    }
    num = cursor.read<uint32_t>();
    std::pmr::vector<FPropertyValue> vec(cursor.m_owner->storage());
    vec.reserve(num);
    for (int i = 0; i < num; i++) {
      vec.emplace_back(parsePropertyValue(cursor, storage, tag, ReadType::ARRAY, 0));
    }

    return {.data = std::move(vec)};
  }

  template<>
  FPropertyValue PropertyParser<EUE5Version::UE5_3>::parseBoolPropertyValue(
    ByteCursor &cursor, sc::MonotonicResource *storage, FPropertyTagData *tagData, ReadType type) {
    FPropertyValue val;
    switch (type) {
      case ReadType::NORMAL:
        if (tagData->BoolValue.has_value()) {
          val.data = tagData->BoolValue.value();
          break;
        }
      case ReadType::MAP:
      case ReadType::ARRAY:
        val.data = cursor.readFlag();
        break;
    }
    return val;
  }

  template<>
  void PropertyParser<EUE5Version::UE5_3>::parse(PropertyHolder *holder, ByteCursor &cursor,
                                                 sc::MonotonicResource *storage) {
    PropertyMapBuilder properties;
    while (true) {
      auto tag = parsePropertyTag(cursor, storage);
      if (tag.Name.isNone()) {
        holder->m_properties = properties.build(cursor.m_owner->storage());
        return;
      }
      auto final = cursor.tell() + tag.Size;
      tag.Tag = parsePropertyValue(cursor, storage, tag, ReadType::NORMAL, tag.Size);
      cursor = final;
      properties.add(PropertyHolder::propertyKey(sc::hash(tag.Name.text()), tag.ArrayIndex), std::move(tag));
    }
  }
}
