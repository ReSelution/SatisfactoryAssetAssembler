#include "UsmapReader.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Decompressor.hpp"
#include "logger.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAP {
  static constexpr uint16_t USMAP_MAGIC = 0x30C4;

  UsmapReader::UsmapReader(std::filesystem::path path) : m_Path(std::move(path)) {}

  bool UsmapReader::parse(FUsmapMappingData &outputData) {
    std::ifstream file(m_Path, std::ios::binary);
    if (!file.is_open()) {
      Logger::logError("[USMAP]", "Konnte Datei nicht öffnen: {}", m_Path.string());
      return false;
    }

    try {
      // 1. Header lesen
      if (!readHeader(file, outputData)) {
        return false;
      }

      // 2. Dekomprimieren
      if (!readAndDecompress(file)) {
        return false;
      }
      file.close(); // Datei wird nicht mehr benötigt

      // 3. Payload Parsen
      size_t cursor = 0;
      parseNameMap(cursor, outputData);
      parseEnums(cursor, outputData);
      parseSchemas(cursor, outputData);
      return true;
    } catch (const std::exception &e) {
      Logger::logError("[USMAP]", "Exception während des Parsings: {}", e.what());
      return false;
    }
  }

  // -------------------------------------------------------------------------
  // Header & Compression
  // -------------------------------------------------------------------------

  bool UsmapReader::readHeader(std::ifstream &file, FUsmapMappingData &outputData) {
    uint16_t magic = 0;
    file.read(reinterpret_cast<char *>(&magic), sizeof(magic));
    if (magic != USMAP_MAGIC) {
      Logger::logError("[USMAP]", "Ungültige Magic Number: {:#x}", magic);
      return false;
    }

    file.read(reinterpret_cast<char *>(&m_version), sizeof(m_version));
    if (!file)
      throw std::runtime_error("EOF bei Version");

    if (m_version > EUsmapVersion::LATEST) {
      throw std::runtime_error("Ungültige USMAP Version (zu neu?)");
    }

    bool bHasVersioning = false;
    if (m_version >= EUsmapVersion::PACKAGE_VERSIONING) {
      uint32_t flag = 0;
      file.read(reinterpret_cast<char *>(&flag), sizeof(flag));
      bHasVersioning = (flag != 0);
    }

    if (bHasVersioning) {
      FPackageFileVersion &pkg = outputData.pkgVersion;
      file.read(reinterpret_cast<char *>(&pkg), sizeof(pkg)); // 8 Bytes

      uint32_t cvCount = 0;
      file.read(reinterpret_cast<char *>(&cvCount), sizeof(cvCount)); // 4 Bytes

      // Custom Versions überspringen (20 Bytes pro Eintrag)
      file.seekg(static_cast<std::streamoff>(cvCount) * 20, std::ios::cur);

      uint32_t netCL = 0;
      file.read(reinterpret_cast<char *>(&netCL), sizeof(netCL)); // 4 Bytes
      Logger::logError("[USMAP]", "Version: UE4: {} UE5: {}", pkg.FileVersionUE4,
                       static_cast<uint32_t>(pkg.FileVersionUE5));
    }

    // Kompressionsinfos
    file.read(reinterpret_cast<char *>(&m_compressionMethod), sizeof(m_compressionMethod));
    file.read(reinterpret_cast<char *>(&m_compressedSize), sizeof(m_compressedSize));
    file.read(reinterpret_cast<char *>(&m_decompressedSize), sizeof(m_decompressedSize));

    if (!file)
      return false;

    return true;
  }

  bool UsmapReader::readAndDecompress(std::ifstream &file) {
    if (m_compressedSize == 0)
      return false;

    std::vector<uint8_t> compressedData(m_compressedSize);
    file.read(reinterpret_cast<char *>(compressedData.data()), m_compressedSize);
    if (file.gcount() != static_cast<std::streamsize>(m_compressedSize)) {
      Logger::logError("[USMAP]", "Konnte komprimierte Daten nicht vollständig lesen.");
      return false;
    }

    m_Buffer.resize(m_decompressedSize);

    // Ruft den Wrapper in Decompressor.cpp auf
    return Decompressor::decompressUsmapPayload(compressedData, m_Buffer, m_compressionMethod);
  }

  // -------------------------------------------------------------------------
  // Parsing Logic
  // -------------------------------------------------------------------------

  void UsmapReader::parseNameMap(size_t &cursor, FUsmapMappingData &outData) {
    uint32_t nameCount = readUInt32(cursor);
    m_TempNameMap.resize(nameCount);

    for (uint32_t i = 0; i < nameCount; ++i) {
      uint8_t len = readByte(cursor);
      m_TempNameMap[i] = std::string_view(reinterpret_cast<const char *>(m_Buffer.data() + cursor), len);
      cursor += len;
    }
    outData.NameMap = m_TempNameMap;
  }

  void UsmapReader::parseEnums(size_t &cursor, FUsmapMappingData &outData) {
    uint32_t enumCount = readUInt32(cursor);
    outData.Enums.clear();
    outData.Enums.reserve(enumCount);

    for (uint32_t i = 0; i < enumCount; ++i) {
      std::string_view enumName = readName(cursor);
      auto values = readEnumValues(cursor);

      outData.Enums.emplace(std::move(std::string(enumName)), std::move(values));
    }
  }

  std::vector<std::string_view> UsmapReader::readEnumValues(size_t &cursor) {
    std::vector<std::string_view> values;

    uint16_t count = 0;
    if (m_version >= EUsmapVersion::LARGE_ENUMS) {
      count = readUInt16(cursor);
    } else {
      count = readByte(cursor);
    }

    if (m_version >= EUsmapVersion::EXPLICIT_ENUM_VALUES) {
      for (uint16_t i = 0; i < count; ++i) {
        uint64_t val = readUInt64(cursor);
        auto name = readName(cursor);

        if (val >= values.size()) {
          values.resize(static_cast<size_t>(val) + 1);
        }
        values[static_cast<size_t>(val)] = name;
      }
    } else {
      values.reserve(count);
      for (uint16_t i = 0; i < count; ++i) {
        values.push_back(readName(cursor));
      }
    }
    return values;
  }

  void UsmapReader::parseSchemas(size_t &cursor, FUsmapMappingData &outData) {
    uint32_t schemaCount = readUInt32(cursor);
    outData.Schemas.clear();
    outData.Schemas.reserve(schemaCount);

    for (uint32_t i = 0; i < schemaCount; ++i) {
      FUsmapSchema schema = readSingleSchema(cursor);
      outData.Schemas.emplace(schema.Name, std::move(schema));
    }
  }

  FUsmapSchema UsmapReader::readSingleSchema(size_t &cursor) {
    FUsmapSchema schema;
    schema.Name = readName(cursor);
    schema.SuperName = readName(cursor);

    uint16_t propCount = readUInt16(cursor); // Ignoriert (total count)
    uint16_t serializablePropCount = readUInt16(cursor);

    for (uint16_t i = 0; i < serializablePropCount; ++i) {
      readAndAddProperty(cursor, schema);
    }
    return schema;
  }

  void UsmapReader::readAndAddProperty(size_t &cursor, FUsmapSchema &schema) {
    // 1. Info Header lesen
    uint16_t schemaIndex = readUInt16(cursor);
    uint8_t arraySize = readByte(cursor);
    std::string_view propName = readName(cursor);

    // 2. Typ rekursiv parsen
    auto typeData = parsePropertyType(cursor);

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
      // Der Index wird hochgezählt
      instance.SchemaIndex = static_cast<uint16_t>(schemaIndex + k);

      schema.Properties[instance.SchemaIndex] = std::move(instance);
    }
  }

  std::shared_ptr<FUsmapPropertyData> UsmapReader::parsePropertyType(size_t &cursor) {
    auto data = std::make_shared<FUsmapPropertyData>();

    data->Type = static_cast<EUsmapPropertyType>(readByte(cursor));

    switch (data->Type) {
      case EUsmapPropertyType::ENUM_PROPERTY:
        data->InnerType = parsePropertyType(cursor);
        data->EnumName = readName(cursor);
        break;
      case EUsmapPropertyType::STRUCT_PROPERTY:
        data->StructName = readName(cursor);
        break;
      case EUsmapPropertyType::SET_PROPERTY:
      case EUsmapPropertyType::ARRAY_PROPERTY:
      case EUsmapPropertyType::OPTIONAL_PROPERTY:
        data->InnerType = parsePropertyType(cursor);
        break;
      case EUsmapPropertyType::MAP_PROPERTY:
        data->InnerType = parsePropertyType(cursor);
        data->ValueType = parsePropertyType(cursor);
        break;
      default:
        // Einfache Typen haben keine Metadaten
        break;
    }
    return data;
  }

  // -------------------------------------------------------------------------
  // Primitive Readers (mit Safety Checks)
  // -------------------------------------------------------------------------

  void UsmapReader::checkBounds(size_t cursor, size_t size) const {
    if (cursor + size > m_Buffer.size()) {
      throw std::out_of_range("Buffer Overflow beim Lesen der USMAP Daten");
    }
  }

  uint8_t UsmapReader::readByte(size_t &cursor) {
    checkBounds(cursor, 1);
    return m_Buffer[cursor++];
  }

  uint16_t UsmapReader::readUInt16(size_t &cursor) {
    checkBounds(cursor, sizeof(uint16_t));
    uint16_t v;
    std::memcpy(&v, m_Buffer.data() + cursor, sizeof(v));
    cursor += sizeof(v);
    return v;
  }

  uint32_t UsmapReader::readUInt32(size_t &cursor) {
    checkBounds(cursor, sizeof(uint32_t));
    uint32_t v;
    std::memcpy(&v, m_Buffer.data() + cursor, sizeof(v));
    cursor += sizeof(v);
    return v;
  }

  uint64_t UsmapReader::readUInt64(size_t &cursor) {
    checkBounds(cursor, sizeof(uint64_t));
    uint64_t v;
    std::memcpy(&v, m_Buffer.data() + cursor, sizeof(v));
    cursor += sizeof(v);
    return v;
  }

  std::string_view UsmapReader::readName(size_t &cursor) {
    uint32_t index = readUInt32(cursor);

    if (index == 0xFFFFFFFFU)
      return ""; // Null Name

    if (index >= m_TempNameMap.size()) {
      throw std::runtime_error("NameMap Index OutOfBounds: " + std::to_string(index));
    }
    return m_TempNameMap[index];
  }
} // namespace SAP
