#include "games/satisfactory/SatisfactoryDataProvider.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <future>
#include <ranges>

#include "OodleEmbedded.hpp"
#include "IOStoreReader.hpp"
#include "PAKReader.hpp"
#include "Reader.hpp"
#include "UsmapReader.hpp"
#include "logger.hpp"
#include "Decompressor.hpp"
#include "AssetParser.hpp"
#include "io_global_data.hpp"
#ifdef _WIN32
#include <windows.h>
#define OodleHandle HMODULE
#else
#include <dlfcn.h>
#define OodleHandle void *
#endif

namespace SAP {
  static constexpr const char *LOGGER_PROVIDER = "[Provider]";
  static constexpr const char *LOGGER_OODLE = "[Oodle]";

  SatisfactoryDataProvider::SatisfactoryDataProvider(const std::filesystem::path &baseInstallPath) {
    Logger::Logger::getInstance().initLogger(LOGGER_PROVIDER);
    Logger::Logger::getInstance().initLogger(LOGGER_OODLE);

    Logger::logInfo(LOGGER_PROVIDER, "Starte Datei-Suche in {}", baseInstallPath.string());

    findAllParsableFiles(baseInstallPath);

    Logger::logInfo(LOGGER_PROVIDER, "Gefundene Parser: {}", m_Parsers.size());
    loadOodle();
  }

  bool SatisfactoryDataProvider::load() {
    m_UsmapParser->parse(m_usmapData);
    m_globalDataParser->parse(m_globalData);
    for (const auto &val: m_Parsers | std::views::values) {
      Logger::logInfo(LOGGER_PROVIDER, "Lade Assets from {}", val->getPath().filename().string());
      val->load();
      val->mountVFS(m_vfs);
    }
    return true;
  }

  void SatisfactoryDataProvider::test() {
    Logger::logInfo(LOGGER_PROVIDER, "Starte Test-Parsing...");

    // Beispiel-Pfad
    std::string testPath =
        "FactoryGame/Content/FactoryGame/Buildable/Building/Ladder/UI/IconDesc_Ladder_256.uasset";

    AssetParser parser(&m_vfs, &m_usmapData, &m_globalData);
    parser.queueFile(testPath);
    parser.processQueue
        (
         [&](const AssetParseResult &result) { Logger::logInfo("[Test]", "Finised Parsing"); }, 5);
  }

  void SatisfactoryDataProvider::findAllParsableFiles(const std::filesystem::path &folder) {
    if (!std::filesystem::exists(folder) || !std::filesystem::is_directory(folder)) {
      Logger::logWarn(LOGGER_PROVIDER, "Pfad {} ist ungültig.", folder.string());
      return;
    }
    for (const auto &entry: std::filesystem::recursive_directory_iterator(folder)) {
      if (!entry.is_regular_file()) { continue; }

      auto path = entry.path();
      auto ext = path.extension().string();
      auto basename = path.stem().string();

      if (ext == ".pak") { m_Parsers[basename] = std::make_unique<PAKReader>(path); }

      else if (ext == ".utoc") {
        std::filesystem::path ucasPath = path;
        ucasPath.replace_extension(".ucas");

        if (std::filesystem::exists(ucasPath)) {
          if (path.filename() == "global.utoc") {
            m_globalDataParser = std::make_unique<IOGlobalData>(path, ucasPath);
          }else {
            m_Parsers[basename] = std::make_unique<IOStoreReader>(path, ucasPath);
          }
        }
        else {
          Logger::logWarn(LOGGER_PROVIDER, "UTOC gefunden, aber keine passende UCAS: {}", path.string());
        }
      } else if (ext == ".usmap") { m_UsmapParser = std::make_unique<UsmapReader>(path); }
    }
  }

  // --------------------- OODLE CODE ------------------------
  bool SatisfactoryDataProvider::loadOodle() {
    Logger::logInfo(LOGGER_OODLE, "Lade Oodle…");

#ifdef _WIN32
    const unsigned char *oodleBytes = oo2core_9_win64_dll;
    size_t oodleSize = oo2core_9_win64_dll_size;
#else
    const unsigned char *oodleBytes{};
    size_t oodleSize = 0;
#endif

    std::filesystem::path tempPath = std::filesystem::temp_directory_path() / OODLE_DLL_FILENAME;

    Logger::logDebug(LOGGER_OODLE, "Extrahiere nach {}", tempPath.string());

    std::ofstream tempFile(tempPath, std::ios::binary | std::ios::trunc);
    if (!tempFile.is_open()) {
      Logger::logError(LOGGER_OODLE, "Konnte temporäre Datei nicht erstellen: {}", tempPath.string());
      return false;
    }

    tempFile.write
        (reinterpret_cast<const char *>(oodleBytes), static_cast<uint32_t>(oodleSize));
    tempFile.close();

#ifdef _WIN32
    mOodleHandle = (void *) LoadLibraryA(tempPath.string().c_str());
    if (!mOodleHandle) {
      Logger::logError(LOGGER_OODLE, "Oodle konnte nicht geladen werden. Windows Error: {}", GetLastError());
      return false;
    }
#else
    m_OodleHandle = dlopen(tempPath.string().c_str(), RTLD_LAZY);
    if (m_OodleHandle == nullptr) {
      Logger::logError(LOGGER_OODLE, "dlopen() Fehlgeschlagen: {}", dlerror());
      return false;
    }
#endif

    Logger::logInfo(LOGGER_OODLE, "Oodle erfolgreich geladen.");
    if (!SAP::Decompressor::initialize(m_OodleHandle)) {
      Logger::logError(LOGGER_OODLE, "Fehler beim Extrahieren der Oodle Funktionspointer.");
      return false;
    }
    return true;
  }

  SatisfactoryDataProvider::~SatisfactoryDataProvider() {
    if (m_OodleHandle != nullptr) {
      Logger::logDebug(LOGGER_OODLE, "Entlade Oodle-Bibliothek…");

#ifdef _WIN32
      FreeLibrary((HMODULE) mOodleHandle);
#else
      dlclose(m_OodleHandle);
#endif
    }
  }
} // namespace SAP
