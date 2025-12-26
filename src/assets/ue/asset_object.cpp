//
// Created by oleub on 14.12.25.
//

#include "ue/asset_object.hpp"

#include "uasset_parser.hpp"
#include "functions.hpp"
#include "logger.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAP {
  void Object::parse(size_t &cursor, AssetFileParser *owner) {
    m_owner = owner;
    auto parser = reinterpret_cast<UAssetParser *>(m_owner);
    auto asset = parser->getAsset();
    parseTaggedProperties(asset, cursor);
    bool hasGuid = readPrimitive<uint32_t>(asset, cursor) == 1;
    if (hasGuid) {
      readStruct<FGuid>(asset, cursor);
    }
  }

  FName Object::readFName(const std::vector<uint8_t> &asset, size_t &cursor) {
    auto mappedName = readStruct<FMappedName>(asset, cursor);

    FName name{
      .index = mappedName.nameIndex, .number = mappedName.extraIndex,
      .plainText = reinterpret_cast<UAssetParser *>(m_owner)->nameFromMap(mappedName)
    };
    return name;
  }

  std::string Object::getFullName() const {
    std::string name = m_className;
    name += m_owner->path();
    name += "." + m_objName;
    return name;
  }

  void Object::setObjName(std::string name) { m_objName = std::move(name); }

  void Object::parseTaggedProperties(const std::vector<uint8_t> &asset, size_t &cursor) {
    while (true) {
      auto tag = FPropertyTag{.Name = readFName(asset, cursor)};
      if (tag.Name.isNone()) { return; }
      Logger::logError("EBTRY", "TAG: {}", tag.Name.text());
      if (m_owner->mappings()->pkgVersion.FileVersionUE5 >=
          EUnrealEngineObjectUE5Version::PROPERTY_TAG_COMPLETE_TYPE_NAME) {
        throw std::runtime_error("not implemented: PROPERTY_TAG_COMPLETE_TYPE_NAME");
      } else {
        auto fname = readFName(asset, cursor);
        Logger::logError("ENTY", "TYPE {}", fname.text());
        tag.PropertyType = hashEnum<EPropertyTagType>(fname.text());
        tag.Size = readPrimitive<uint32_t>(asset, cursor);
        tag.ArrayIndex = readPrimitive<uint32_t>(asset, cursor);
        tag.TagData = parsePropertyTagData(asset, cursor, tag.PropertyType);
        auto hasPropertyGuid = readPrimitive<uint8_t>(asset, cursor) == 1;
        if (hasPropertyGuid) { tag.PropertyGuid = readStruct<FGuid>(asset, cursor); }
        if (m_owner->mappings()->pkgVersion.FileVersionUE5 >=
            EUnrealEngineObjectUE5Version::PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION) {
          throw std::runtime_error("not implemented: PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION");
        }
      }

      auto final = cursor + tag.Size;
      tag.Tag = parsePropertyValue(asset, cursor, tag.PropertyType, tag.TagData.get(), ReadType::NORMAL, tag.Size);
      cursor = final;

      m_properties.emplace(fnv1a(tag.Name.text().c_str()), std::move(tag));
    }
  }

  std::unique_ptr<FPropertyValue> Object::parsePropertyValue
  (const std::vector<uint8_t> &asset, size_t &cursor, EPropertyTagType type, const FPropertyTagData *tagData,
   ReadType readType,
   int32_t size) {
    switch (type) {
      case EPropertyTagType::Struct:
        return parseStructPropertyValue(asset, cursor, tagData->StructType, readType);
      case EPropertyTagType::Bool:
        Logger::logError("[OBJ]", "EPropertyTagType::BOOL not Implemented");
        break;
      case EPropertyTagType::Byte:
        return parseBytePropertyValue(asset, cursor, readType);
      case EPropertyTagType::Enum:
        Logger::logError("[OBJ]", "EPropertyTagType::Enum not Implemented");
        break;
      case EPropertyTagType::Array:
        Logger::logError("[OBJ]", "EPropertyTagType::Array not Implemented");
        break;
      case EPropertyTagType::Set:
        Logger::logError("[OBJ]", "EPropertyTagType::Set not Implemented");
        break;
      case EPropertyTagType::Map:
        Logger::logError("[OBJ]", "EPropertyTagType::Map not Implemented");
        break;
      case EPropertyTagType::Optional:
        Logger::logError("[OBJ]", "EPropertyTagType::Optional not Implemented");
        break;

      default:
        break;
    }
    return nullptr;
  }

  std::unique_ptr<FPropertyValue> Object::parseStructPropertyValue
  (const std::vector<uint8_t> &asset, size_t &cursor, std::string structType, ReadType type) {
    auto hash = hashEnum<ESatisfactoryStruct>(structType);
    switch (hash) {
      case ESatisfactoryStruct::Vector: {
        auto val = std::make_unique<FPropertyValue>();
        // val->Data = ... parse FVector from asset + cursor
        return val;
      }

      case ESatisfactoryStruct::Vector2D: {
        auto val = std::make_unique<FPropertyValue>();
        // val->Data = ... parse FVector2D
        return val;
      }

      case ESatisfactoryStruct::Rotator: {
        auto val = std::make_unique<FPropertyValue>();
        // val->Data = ... parse FRotator
        return val;
      }

      case ESatisfactoryStruct::Transform: {
        auto val = std::make_unique<FPropertyValue>();
        // val->Data = ... parse FTransform
        return val;
      }

      case ESatisfactoryStruct::Box: {
        auto val = std::make_unique<FPropertyValue>();
        // val->Data = ... parse FBox
        return val;
      }

      case ESatisfactoryStruct::Guid: {
        auto val = std::make_unique<FPropertyValue>();
        val->Data = readStruct<FGuid>(asset, cursor);
        return val;
      }

      case ESatisfactoryStruct::LinearColor: {
        auto val = std::make_unique<FPropertyValue>();
        // val->Data = ... parse FLinearColor
        return val;
      }

      case ESatisfactoryStruct::IntPoint: {
        auto val = std::make_unique<FPropertyValue>();
        val->Data = readStruct<IntPoint>(asset, cursor);
        Logger::logError("[Point]", "x: {}, y: {}", std::get<IntPoint>(val->Data).x, std::get<IntPoint>(val->Data).y);
        return val;
      }

      default:
        Logger::logError("[OBJ]", "unknown struct type: {}", structType);
        return std::make_unique<FPropertyValue>(); // return empty / fallback
    }
  }

  std::unique_ptr<FPropertyValue> Object::parseBytePropertyValue
  (const std::vector<uint8_t> &asset, size_t &cursor, ReadType type) {
    auto val = std::make_unique<FPropertyValue>();
    val->Data = readStruct<uint8_t>(asset, cursor);
    return val;
  }

  std::unique_ptr<FPropertyTagData> Object::parsePropertyTagData
  (const std::vector<uint8_t> &asset, size_t &cursor, EPropertyTagType type) {
    auto data = std::make_unique<FPropertyTagData>();
    switch (type) {
      case EPropertyTagType::Struct: {
        data->StructType = readFName(asset, cursor).text();
        data->StructGuid = readStruct<FGuid>(asset, cursor);
        break;
      }
      case EPropertyTagType::Bool: {
        data->BoolValue = readPrimitive<uint8_t>(asset, cursor) == 1;
        break;
      }
      case EPropertyTagType::Byte:
      case EPropertyTagType::Enum: {
        data->EnumName = readFName(asset, cursor).text();
        break;
      }
      case EPropertyTagType::Array:
      case EPropertyTagType::Set: {
        data->InnerType = readFName(asset, cursor).text();
        break;
      }
      case EPropertyTagType::Map: {
        data->InnerType = readFName(asset, cursor).text();
        data->ValueType = readFName(asset, cursor).text();
        break;
      }
      case EPropertyTagType::Optional: {
        data->InnerType = readFName(asset, cursor).text();
        break;
      }
      default: {
        Logger::logError("[OBJ]", "unkown type: {}", static_cast<uint32_t>(type));
        break;
      }
    }
    return data;
  }
}
