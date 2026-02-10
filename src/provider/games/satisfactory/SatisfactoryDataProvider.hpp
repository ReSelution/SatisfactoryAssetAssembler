#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "Reader.hpp"
#include "io_global_data.hpp"
#include "UsmapReader.hpp"

#include "vfs.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAP {


  class SatisfactoryDataProvider {
  public:
    SatisfactoryDataProvider(const std::filesystem::path &baseInstallPath);

    ~SatisfactoryDataProvider();

    SatisfactoryDataProvider(const SatisfactoryDataProvider &) = delete;

    SatisfactoryDataProvider(SatisfactoryDataProvider &&) = delete;

    SatisfactoryDataProvider &operator=(const SatisfactoryDataProvider &) = delete;

    SatisfactoryDataProvider &operator=(SatisfactoryDataProvider &&) = delete;

    bool load();
    void crawl();
    void test();

  private:
    Vfs m_vfs;
    std::unique_ptr<UsmapReader> m_UsmapParser = nullptr;
    std::unique_ptr<IOGlobalData> m_globalDataParser = nullptr;
    std::unordered_map<std::string, std::unique_ptr<Reader> > m_Parsers;

    void findAllParsableFiles(const std::filesystem::path &folder);

    FUsmapMappingData m_usmapData;
    GlobalData m_globalData;
  };
} // namespace SAP
