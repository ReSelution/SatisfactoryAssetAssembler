#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "usmap/enums.hpp"

namespace SAP {
  struct ParseContext;

  // Falls UsmapStructs.hpp diese nicht definiert, hier Forward Declarations oder Includes:
  struct FUsmapMappingData;
  struct FUsmapPropertyData;
  struct FUsmapSchema;

  class UsmapReader {
  public:
    explicit UsmapReader(std::filesystem::path path);
    bool parse(FUsmapMappingData &outputData);

  private:
    // --- Haupt-Phasen des Parsings ---
    bool readHeader(std::ifstream &file, FUsmapMappingData &outputData);
    bool readAndDecompress(std::ifstream &file);

    // --- Parsing der Datenblöcke ---
    void parseNameMap(size_t &cursor, FUsmapMappingData &outData);
    void parseEnums(size_t &cursor, FUsmapMappingData &outData);
    void parseSchemas(size_t &cursor, FUsmapMappingData &outData);

    std::vector<std::string_view> readEnumValues(size_t &cursor);

    FUsmapSchema readSingleSchema(size_t &cursor);

    void readAndAddProperty(size_t &cursor, FUsmapSchema &schema);

    std::shared_ptr<FUsmapPropertyData> parsePropertyType(size_t &cursor);

    void checkBounds(size_t cursor, size_t size) const;

    uint8_t readByte(size_t &cursor);

    uint16_t readUInt16(size_t &cursor);

    uint32_t readUInt32(size_t &cursor);

    uint64_t readUInt64(size_t &cursor);

    std::string_view readName(size_t &cursor);

    // --- Member Variablen ---
    std::filesystem::path m_Path;
    EUsmapVersion m_version{EUsmapVersion::INITIAL};

    // Puffer für dekomprimierte Daten
    std::vector<uint8_t> m_Buffer;
    std::vector<std::string_view> m_TempNameMap;

    // Header Infos
    EUsmapCompressionMethod m_compressionMethod{EUsmapCompressionMethod::NONE};
    uint32_t m_compressedSize = 0;
    uint32_t m_decompressedSize = 0;
  };

} // namespace SAP
