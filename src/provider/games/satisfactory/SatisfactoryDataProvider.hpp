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

  class IOGlobalData;
  using OodleLZ_Decompress_Signature = size_t (*)
  (const void *compressedData,
   size_t compressedSize,
   void *decompressedData,
   size_t decompressedSize,
   int flags,
   // Z.B. Oodle::Fidelity::k_Decode_Speed
   size_t fp,
   size_t fpLen,
   void *decodeBuf,
   size_t decodeBufSize,
   int threadModule);

  class SatisfactoryDataProvider {
  public:
    SatisfactoryDataProvider(const std::filesystem::path &baseInstallPath);

    ~SatisfactoryDataProvider();

    SatisfactoryDataProvider(const SatisfactoryDataProvider &) = delete;

    SatisfactoryDataProvider(SatisfactoryDataProvider &&) = delete;

    SatisfactoryDataProvider &operator=(const SatisfactoryDataProvider &) = delete;

    SatisfactoryDataProvider &operator=(SatisfactoryDataProvider &&) = delete;

    bool load();

    void test();

  private:
    Vfs m_vfs;
    std::unique_ptr<UsmapReader> m_UsmapParser = nullptr;
    std::unique_ptr<IOGlobalData> m_globalDataParser = nullptr;
    std::unordered_map<std::string, std::unique_ptr<Reader> > m_Parsers;

    void findAllParsableFiles(const std::filesystem::path &folder);

    FUsmapMappingData m_usmapData;
    GlobalData m_globalData;

    // oodle stuff
    bool loadOodle();

    OodleLZ_Decompress_Signature m_OodleDecompressFunc = nullptr;
    void *m_OodleHandle = nullptr;
  };
} // namespace SAP
