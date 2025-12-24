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


    // Liest die Enum-Namen (abhängig von der Version)
    std::vector<std::string> readEnumValues(size_t &cursor);

    // Liest eine einzelne Schema-Definition (Header + Properties)
    FUsmapSchema readSingleSchema(size_t &cursor);

    // Liest eine Property-Definition (Index, Dim, Name, Typ)
    // und fügt sie (evtl. mehrfach bei Arrays) zum Schema hinzu.
    void readAndAddProperty(size_t &cursor, FUsmapSchema &schema);

    // Rekursives Parsen des Typs (ohne Index/Name)
    std::shared_ptr<FUsmapPropertyData> parsePropertyType(size_t &cursor);

    // --- Low-Level Reader mit Bounds-Check ---
    void checkBounds(size_t cursor, size_t size) const;

    uint8_t readByte(size_t &cursor);

    uint16_t readUInt16(size_t &cursor);

    uint32_t readUInt32(size_t &cursor);

    uint64_t readUInt64(size_t &cursor);

    std::string readName(size_t &cursor);

    // --- Member Variablen ---
    std::filesystem::path m_Path;
    EUsmapVersion m_version{ EUsmapVersion::INITIAL };

    // Puffer für dekomprimierte Daten
    std::vector<uint8_t> m_Buffer;
    // Temporäre Namenstabelle während des Parsings
    std::vector<std::string> m_TempNameMap;

    // Header Infos
    EUsmapCompressionMethod m_compressionMethod{ EUsmapCompressionMethod::NONE };
    uint32_t m_compressedSize = 0;
    uint32_t m_decompressedSize = 0;
  };

} // namespace SAP