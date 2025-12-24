//
// Created by oleub on 07.12.25.
//

#pragma once
#include <cstdint>
#include <utility>
#include <vector>

#include "AssetParser.hpp"
#include "io/packages/f_file_summary.hpp"
#include "io/packages/f_object_export.hpp"
#include "io/packages/f_object_import.hpp"

namespace SAP {

  struct AssetParseResult {
    std::filesystem::path FilePath;
    bool bSuccess = false;

    std::vector<std::string> NameMap;
    std::vector<FObjectImport> Imports;
    std::vector<FObjectExport> Exports;
    FFileSummary Summary{};
  };


  class AssetFileParser {
  public:
    enum class AssetType {
      UASSET,
      UMAP,
      UNKNOWN
    };

    AssetFileParser
    (AssetParser *assetParser, std::string path , AssetType assetType = AssetType::UNKNOWN ) : m_assetParser(assetParser), m_type(assetType), m_path(std::move(path)) {}


    virtual ~AssetFileParser() = default;

    [[nodiscard]] const AssetType type();
    virtual AssetParseResult parse(const std::vector<uint8_t> &asset, const std::vector<uint8_t> &bulk = {}) = 0;
    [[nodiscard]] const FUsmapMappingData* mappings() const { return m_assetParser->mappings(); }
    [[nodiscard]] const GlobalData* globalData() const { return m_assetParser->globalData(); }
    std::string path() {return m_path;}
  protected:
    AssetParser *m_assetParser;

  private:
    AssetType m_type;;
    std::string m_path;
  };
}
