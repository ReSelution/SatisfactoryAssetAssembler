#include "UsmapReader.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "byte_cursor.hpp"
#include "Decompressor.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

#include "sc/logger/logger.hpp"

using UsmapLog = sc::Logger<"SAA", "USAMP">;

namespace SAA {
  static constexpr uint16_t USMAP_MAGIC = 0x30C4;

  UsmapReader::UsmapReader(std::filesystem::path path) : m_Path(std::move(path)) {
  }

  bool UsmapReader::parse(FUsmapMappingData &outputData) {
    auto enumSize = outputData.Enums.size();
    auto schemasSize = outputData.Schemas.size();
    auto t = UsmapLog::time("Parsed {1} Enums and {2} Schemas in {0}", enumSize,
                            schemasSize);
    std::ifstream file(m_Path, std::ios::binary);

    if (!file.is_open()) {
      UsmapLog::err("Konnte Datei nicht öffnen: {}", m_Path.string());
      return false;
    }
    std::vector<uint8_t> buffer;
    file.seekg(0, std::ios::end);
    buffer.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
    file.close();
    ByteCursor rawCursor(buffer);
    if (!readHeader(rawCursor, outputData)) {
      return false;
    }


    // Doesnt work with the 5.6 Game version of Statisfactory
    // The only data that is needed is the game version

    // if (!readAndDecompress(rawCursor, outputData)) {
    //   return false;
    // }
    //
    // ByteCursor cursor(m_data);
    //
    // parseNameMap(cursor, outputData);
    // parseEnums(cursor, outputData);
    // parseSchemas(cursor, outputData);
    // enumSize = outputData.Enums.size();
    // schemasSize = outputData.Schemas.size();
    return true;
  }

  // -------------------------------------------------------------------------
  // Header & Compression
  // -------------------------------------------------------------------------

  bool UsmapReader::readHeader(ByteCursor &cursor, FUsmapMappingData &outputData) {
    uint16_t magic = cursor.read<uint16_t>();
    if (magic != USMAP_MAGIC) {
      UsmapLog::err("Ungültige Magic Number: {:#x}", magic);
      return false;
    }

    m_version = cursor.read<EUsmapVersion>();

    if (m_version > EUsmapVersion::LATEST) {
      throw std::runtime_error("Ungültige USMAP Version (zu neu?)");
    }

    bool bHasVersioning = false;
    if (m_version >= EUsmapVersion::PACKAGE_VERSIONING) {
      uint32_t flag = cursor.read<uint32_t>();
      bHasVersioning = (flag != 0);
    }

    if (bHasVersioning) {
      outputData.pkgVersion = cursor.read<FPackageFileVersion>();

      uint32_t cvCount = cursor.read<uint32_t>();
      cursor += cvCount * 20;

      uint32_t netCL = cursor.read<uint32_t>();
      UsmapLog::err("Version: UE4: {} UE5: {}", outputData.pkgVersion.FileVersionUE4,
                    static_cast<uint32_t>(outputData.pkgVersion.FileVersionUE5));
    }


    return true;
  }

  bool UsmapReader::readAndDecompress(ByteCursor &cursor, FUsmapMappingData &outData) {
    auto compressionMethod = cursor.read<EUsmapCompressionMethod>();
    uint32_t compressedSize = cursor.read<uint32_t>();
    uint32_t decompressedSize = cursor.read<uint32_t>();

    m_data = outData.storage.allocateSpan(decompressedSize);
    auto subspan = cursor.subspan(cursor.tell(), compressedSize);

    return Decompressor::decompressUsmapPayload(subspan.buffer(), m_data, compressionMethod);
  }

  // -------------------------------------------------------------------------
  // Parsing Logic
  // -------------------------------------------------------------------------

  void UsmapReader::parseNameMap(ByteCursor &cursor, FUsmapMappingData &outData) {
    uint32_t nameCount = cursor.read<uint32_t>();
    m_tempNameMap = outData.storage.allocateSpan<std::string_view>(nameCount);
    for (uint32_t i = 0; i < nameCount; ++i) {
      m_tempNameMap[i] = cursor.readStrView();
    }
    outData.NameMap = m_tempNameMap;
  }

  void UsmapReader::parseEnums(ByteCursor &cursor, FUsmapMappingData &outData) {
    uint32_t enumCount = cursor.read<uint32_t>();
    using denseMapEnums = ankerl::unordered_dense::pmr::map<uint64_t, std::span<std::string_view>, sc::TransparentHash>;
    outData.Enums = denseMapEnums(&outData.storage);
    outData.Enums.reserve(enumCount);

    for (uint32_t i = 0; i < enumCount; ++i) {
      std::string_view enumName = readName(cursor);
      auto values = readEnumValues(cursor, outData);

      outData.Enums.emplace(sc::hash(enumName), std::move(values));
    }
  }

  std::span<std::string_view> UsmapReader::readEnumValues(ByteCursor &cursor, FUsmapMappingData &outData) {
    uint16_t count = 0;
    if (m_version >= EUsmapVersion::LARGE_ENUMS) {
      count = cursor.read<uint16_t>();
    } else {
      count = cursor.read<uint8_t>();
    }

    auto enumValues = outData.storage.allocateSpan<std::string_view>(count);


    if (m_version >= EUsmapVersion::EXPLICIT_ENUM_VALUES) {
      for (uint16_t i = 0; i < count; ++i) {
        uint64_t val = cursor.read<uint64_t>();
        auto name = readName(cursor);
        enumValues[val] = name;
      }
    } else {
      for (uint16_t i = 0; i < count; ++i) {
        enumValues[i] = readName(cursor);
      }
    }
    return enumValues;
  }

  void UsmapReader::parseSchemas(ByteCursor &cursor, FUsmapMappingData &outData) {
    uint32_t schemaCount = cursor.read<uint32_t>();
    using denseMapSchemas = ankerl::unordered_dense::pmr::map<uint64_t, FUsmapSchema, sc::TransparentHash>;
    outData.Schemas = denseMapSchemas (&outData.storage);
    outData.Schemas.reserve(schemaCount);

    for (uint32_t i = 0; i < schemaCount; ++i) {
      FUsmapSchema schema = readSingleSchema(cursor, outData);
      outData.Schemas.emplace(sc::hash(schema.Name), std::move(schema));
    }
  }

  FUsmapSchema UsmapReader::readSingleSchema(ByteCursor &cursor, FUsmapMappingData &outData) {
    FUsmapSchema schema;
    schema.Name = readName(cursor);
    schema.SuperName = readName(cursor);

    uint16_t propCount = cursor.read<uint16_t>(); // Ignoriert (total count)
    uint16_t serializablePropCount = cursor.read<uint16_t>();
    schema.Properties = std::pmr::unordered_map<uint16_t, FUsmapPropertyData>(&outData.storage);
    schema.Properties.reserve(serializablePropCount);
    for (uint16_t i = 0; i < serializablePropCount; ++i) {
      readAndAddProperty(cursor, outData, schema);
    }
    return schema;
  }

  void UsmapReader::readAndAddProperty(ByteCursor &cursor, FUsmapMappingData &outData, FUsmapSchema &schema) {
    uint16_t schemaIndex = cursor.read<uint16_t>();
    uint8_t arraySize = cursor.read<uint8_t>();
    std::string_view propName = readName(cursor);

    auto typeData = parsePropertyType(cursor, outData);

    // 3. Basis-Property erstellen
    FUsmapPropertyData baseProp;
    baseProp.Name = propName;
    baseProp.SchemaIndex = schemaIndex;
    baseProp.ArraySize = arraySize;

    baseProp.Type = typeData->Type;
    baseProp.StructName = typeData->StructName;
    baseProp.EnumName = typeData->EnumName;
    baseProp.InnerType = typeData->InnerType;
    baseProp.ValueType = typeData->ValueType;

    for (uint8_t k = 0; k < arraySize; ++k) {
      FUsmapPropertyData instance = baseProp;
      instance.SchemaIndex = static_cast<uint16_t>(schemaIndex + k);
      schema.Properties[instance.SchemaIndex] = std::move(instance);
    }
  }

  FUsmapPropertyData *UsmapReader::parsePropertyType(ByteCursor &cursor, FUsmapMappingData &outData) {
    auto data = outData.storage.make<FUsmapPropertyData>();


    data->Type = cursor.read<EUsmapPropertyType>();

    switch (data->Type) {
      case EUsmapPropertyType::ENUM_PROPERTY:
        data->InnerType = parsePropertyType(cursor, outData);
        data->EnumName = readName(cursor);
        break;
      case EUsmapPropertyType::STRUCT_PROPERTY:
        data->StructName = readName(cursor);
        break;
      case EUsmapPropertyType::SET_PROPERTY:
      case EUsmapPropertyType::ARRAY_PROPERTY:
      case EUsmapPropertyType::OPTIONAL_PROPERTY:
        data->InnerType = parsePropertyType(cursor, outData);
        break;
      case EUsmapPropertyType::MAP_PROPERTY:
        data->InnerType = parsePropertyType(cursor, outData);
        data->ValueType = parsePropertyType(cursor, outData);
        break;
      default:
        break;
    }
    return data;
  }


  std::string_view UsmapReader::readName(ByteCursor &cursor) {
    auto index = cursor.read<uint32_t>();
    if (index == 0xFFFFFFFFU)
      return ""; // Null Name

    if (index >= m_tempNameMap.size()) {
      UsmapLog::err("Namemap index out of bounds: {}",index);
      return "";
    }
    return m_tempNameMap[index];
  }
} 
