#pragma once

#include <filesystem>
#include <span>

#include "sc/memory/monotonic_resource.hpp"
#include "usmap/enums.hpp"

namespace SAA {
  class ByteCursor;
  struct ParseContext;
  struct FUsmapMappingData;
  struct FUsmapPropertyData;
  struct FUsmapSchema;

  class UsmapReader {
  public:
    explicit UsmapReader(std::filesystem::path path);

    bool parse(FUsmapMappingData &outputData);

  private:
    // --- Haupt-Phasen des Parsings ---
    bool readHeader(ByteCursor &cursor, FUsmapMappingData &outputData);

    bool readAndDecompress(ByteCursor &cursor, FUsmapMappingData &outData);

    // --- Parsing der Datenblöcke ---
    void parseNameMap(ByteCursor &cursor, FUsmapMappingData &outData);

    void parseEnums(ByteCursor &cursor, FUsmapMappingData &outData);

    void parseSchemas(ByteCursor &cursor, FUsmapMappingData &outData);

    std::span<std::string_view> readEnumValues(ByteCursor &cursor, FUsmapMappingData &outData);

    FUsmapSchema readSingleSchema(ByteCursor &cursor, FUsmapMappingData &outData);

    void readAndAddProperty(ByteCursor &cursor, FUsmapMappingData &outData, FUsmapSchema &schema);

    FUsmapPropertyData *parsePropertyType(ByteCursor &cursor, FUsmapMappingData &outData);

    std::string_view readName(ByteCursor &cursor);

    std::filesystem::path m_Path;
    EUsmapVersion m_version{EUsmapVersion::INITIAL};


    std::span<uint8_t> m_data;
    std::span<std::string_view> m_tempNameMap;
  };
} 
