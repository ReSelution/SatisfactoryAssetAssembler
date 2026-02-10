//
// Created by oleub on 09.02.26.
//

#include "property_holder.hpp"

#include "AssetFileParser.hpp"
#include "f_property_tag.hpp"
#include "thread_arena.hpp"
#include "core/f_guid.hpp"
#include "core/hash.hpp"

namespace SAP {
  void PropertyHolder::parseTaggedProperties(ByteCursor &cursor, ReadType readType) {
    while (true) {
      auto tag = parsePropertyTag(cursor);
      if (tag.Name.isNone()) { return; }
      auto final = cursor.tell() + tag.Size;
      tag.Tag = parsePropertyValue(cursor, tag, ReadType::NORMAL, tag.Size);
      tag.TagData = nullptr; // ThreadArena Memory gets released after Parsing
      cursor = final;
      m_properties.emplace(fnv1a(tag.Name.text()), std::move(tag));
    }
  }

  void PropertyHolder::releaseResources() { ThreadArena<>::reset(); }

  FPropertyTag PropertyHolder::parsePropertyTag(ByteCursor &cursor) {
    auto tag = FPropertyTag{.Name = cursor.readFName()};
    if (tag.Name.isNone()) { return tag; }

    tag.PropertyTypeName = cursor.readFName();
    //Logger::logInfo("[Property]", "Property {} @ {}", tag.PropertyTypeName.text(), cursor.tell());
    tag.PropertyType = hashEnum<EPropertyTagType>(tag.PropertyTypeName.text());
    tag.Size = cursor.read<uint32_t>();
    tag.ArrayIndex = cursor.read<uint32_t>();
    tag.TagData = parsePropertyTagData(cursor, tag.PropertyType);
    auto hasPropertyGuid = cursor.readFlag();
    if (hasPropertyGuid) { tag.PropertyGuid = cursor.read<FGuid>(); }
    return tag;
  }

  FPropertyValue PropertyHolder::parseMapPropterty
  (ByteCursor &cursor, FPropertyTagData *tagData) {

    FPropertyTag keyTag{.PropertyType = tagData->InnerType, .TagData = tagData->InnerTypeData};
    FPropertyTag valueTag{.PropertyType = tagData->ValueType, .TagData = tagData->ValueTypeData};
    auto keysToRemove = cursor.read<int32_t>();
    for (auto i = 0; i < keysToRemove; i++) { parsePropertyValue(cursor, keyTag, ReadType::MAP, 0); }

    std::vector<std::pair<FPropertyValue, FPropertyValue>> vec;
    auto entries = cursor.read<uint32_t>();
    vec.reserve(entries);
    for (uint32_t i = 0; i < entries; i++) {
      vec.emplace_back
          (
           parsePropertyValue(cursor, keyTag, ReadType::MAP, 0),
           parsePropertyValue(cursor, valueTag, ReadType::MAP, 0)
          );
    }

    return {.data = std::move(vec)};
  }

  FPropertyTagData *PropertyHolder::parsePropertyTagData(ByteCursor &cursor, EPropertyTagType type, ReadType readType) {
    auto *data = ThreadArena<>::make<FPropertyTagData>();
    switch (type) {
      case EPropertyTagType::Struct: {
        auto structType = cursor.readFName();
        data->StructType = hashEnum<ESatisfactoryStruct>(structType.text());
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
        data->InnerType = hashEnum<EPropertyTagType>(cursor.readFName().text());
        break;
      }
      case EPropertyTagType::Map: {
        data->InnerType = hashEnum<EPropertyTagType>(cursor.readFName().text());
        data->ValueType = hashEnum<EPropertyTagType>(cursor.readFName().text());
        break;
      }
      case EPropertyTagType::Optional: {
        data->InnerType = hashEnum<EPropertyTagType>(cursor.readFName().text());
        break;
      }
      default:
        break;
    }
    return data;
  }

  FPropertyValue PropertyHolder::parsePropertyValue
  (ByteCursor &cursor, FPropertyTag &tag, ReadType readType, uint32_t size) {
    FPropertyValue val;
    auto type = tag.PropertyType;
    switch (type) {
      case EPropertyTagType::Struct: {
        val = parseStructPropertyValue(cursor, tag.TagData, readType, size);
        break;
      }
      case EPropertyTagType::Array: {
        val = parseArrayPropertyValue(cursor, tag.TagData, readType);
        break;
      }
      case EPropertyTagType::Map: {
        val = parseMapPropterty(cursor, tag.TagData);
        break;
      }

      case EPropertyTagType::Bool: {
        val = parseBoolPropertyValue(cursor, tag.TagData, readType);
        break;
      }
      case EPropertyTagType::Byte: {
        if (tag.TagData && tag.TagData->EnumName.isNone()) {
          val.data = cursor.read<uint8_t>();
          break;
        }
      }
      case EPropertyTagType::Enum: {
        val.data = cursor.readFName();
        break;
      }
      case EPropertyTagType::Text: {
        val = parseTextPropertyValue(cursor);
        break;
      }
      case EPropertyTagType::Interface:
      case EPropertyTagType::Object: {
        val = parseObjectPropertyValue(cursor);
        break;
      }
      case EPropertyTagType::Set: {
        val = parseSetPropertyValue(cursor, tag.TagData, readType);
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
        val.data = cursor.readFString(m_stringStorage);
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

      case EPropertyTagType::SoftObject:
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
        if (tag.PropertyTypeName.plainTextView.empty()) {
          throw std::runtime_error
              ("unknown property tag: " + to_string(tag.PropertyType) + " @ " + std::to_string(cursor.tell()));
        }
        throw std::runtime_error
            ("unknown property tag: " + std::string(tag.PropertyTypeName.text()) + " @ " + std::to_string
             (cursor.tell()));
    }
    return val;
  }

  FPropertyValue PropertyHolder::parseStructPropertyValue
  (ByteCursor &cursor, FPropertyTagData *tagData, ReadType type, uint32_t size) {
    FPropertyValue val;
    const auto structType = tagData != nullptr ? tagData->StructType : ESatisfactoryStruct::Unknown;
    switch (structType) {
      case ESatisfactoryStruct::Guid: {
        val.data = cursor.read<FGuid>();
        break;
      }
      case ESatisfactoryStruct::IntPoint: {
        val.data = cursor.read<IntPoint>();
        break;
      }
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
      case ESatisfactoryStruct::MovieSceneFrameRange:
      case ESatisfactoryStruct::InventoryItem:
      case ESatisfactoryStruct::NiagaraUserRedirectionParameterStore: {
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
          cursor.readFName(),
          cursor.readBool(),
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
        auto packageName = cursor.readFName();
        auto assetName = cursor.readFName();
        auto array = cursor.readArray<uint8_t>();
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
        val.data = parseFallbackStruct(cursor);
        break;
    }
    return val;
  }

  FPropertyValue PropertyHolder::parseTextPropertyValue(ByteCursor &cursor) {
    FText text{};
    text.flags = cursor.read<ETextFlag>();
    text.type = cursor.read<ETextHistoryType>();

    switch (text.type) {
      case ETextHistoryType::Base: {
        auto nSpace = cursor.readFString(m_stringStorage);
        auto key = cursor.readFString(m_stringStorage);
        auto sourceString = cursor.readFString(m_stringStorage);
        break;
      }
      case ETextHistoryType::StringTableEntry: {
        auto tableId = cursor.readFName();
        auto key = cursor.readFString(m_stringStorage);
        break;
      }
      case ETextHistoryType::NamedFormat:
      case ETextHistoryType::OrderedFormat:
      case ETextHistoryType::ArgumentFormat:
      case ETextHistoryType::AsNumber:
      case ETextHistoryType::AsPercent:
      case ETextHistoryType::AsCurrency:
      case ETextHistoryType::AsDate:
      case ETextHistoryType::AsTime:
      case ETextHistoryType::AsDateTime:
      case ETextHistoryType::Transform:
      case ETextHistoryType::TextGenerator:
        throw std::runtime_error("Not implemented");
      default:
        if (cursor.readBool()) { text.textHistory.text = cursor.readFString(m_stringStorage); }
    }

    return {.data = std::move(text)};
  }

  PropertyHolder PropertyHolder::parseFallbackStruct(ByteCursor &cursor) {
    PropertyHolder structFallback{};
    while (true) {
      auto tag = parsePropertyTag(cursor);
      if (tag.Name.isNone()) { break; };
      tag.Tag = parsePropertyValue(cursor, tag, ReadType::NORMAL,tag.Size);
      structFallback.m_properties.emplace(fnv1a(tag.Name.text()), std::move(tag));
      tag.TagData = nullptr;
    }
    return structFallback;
  }

  FPropertyValue PropertyHolder::parseArrayPropertyValue
  (ByteCursor &cursor, FPropertyTagData *tagData, ReadType type) {
    FPropertyValue val;
    auto arrayType = tagData->InnerType;
    if (arrayType == EPropertyTagType::Unknown)
      return val;

    auto lenght = cursor.read<uint32_t>();
    FPropertyTag tag;
    if (arrayType == EPropertyTagType::Struct) { tag = parsePropertyTag(cursor); }


    std::vector<FPropertyValue> vec;
    vec.reserve(lenght);


    switch (arrayType) {
      case EPropertyTagType::Byte: {
        bool isEnum = cursor.testReadFName();

        if (isEnum) { tag.PropertyType = EPropertyTagType::Enum; } else { tag.PropertyType = EPropertyTagType::Byte; }
        for (int i = 0; i < lenght; i++) {
          vec.emplace_back(parsePropertyValue(cursor, tag, type, tag.Size));
        }
        val.data = std::move(vec);
        return val;
      }
      case EPropertyTagType::Struct: { break; }
      default: { tag.PropertyType = arrayType; }
    }
    type = ReadType::ARRAY;
    for (int i = 0; i < lenght; i++) {
      vec.emplace_back(parsePropertyValue(cursor, tag,  type, tag.Size));
    }
    val.data = std::move(vec);

    return val;
  }

  FPropertyValue PropertyHolder::parseObjectPropertyValue(ByteCursor &cursor) {
    FPropertyValue val;
    auto index = cursor.read<int32_t>();
    val.data = cursor.m_owner->resolveIndex(index);
    return val;
  }

  FPropertyValue PropertyHolder::parseSetPropertyValue(ByteCursor &cursor, FPropertyTagData *tagData, ReadType type) {
    if (tagData->InnerTypeData == nullptr && tagData->InnerType == EPropertyTagType::Struct) {
      if (tagData->Name.text() == "AnimSequenceInstances" || tagData->Name.text() == "PostProcessInstances") {
        tagData->InnerTypeData = ThreadArena<>::make<FPropertyTagData>();
        tagData->InnerTypeData->Type = EPropertyTagType::Struct;
        tagData->InnerTypeData->StructType = static_cast<ESatisfactoryStruct>(fnv1a("Guid"));
      }
    }
    auto num = cursor.read<uint32_t>();
    FPropertyTag tag{
      .PropertyType = tagData->InnerType,
      .TagData = tagData->InnerTypeData,
    };

    for (int i = 0; i < num; i++) { parsePropertyValue(cursor, tag, ReadType::ARRAY, 0); }
    num = cursor.read<uint32_t>();
    std::vector<FPropertyValue> vec;
    vec.reserve(num);
    for (int i = 0; i < num; i++) { vec.emplace_back(parsePropertyValue(cursor, tag, ReadType::ARRAY, 0)); }

    return {.data = std::move(vec)};
  }

  FPropertyValue PropertyHolder::parseBoolPropertyValue
  (ByteCursor &cursor, FPropertyTagData *tagData, ReadType type) {
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
} // SAP
