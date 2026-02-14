//
// Created by oleub on 15.01.26.
//

#include "oodle_loader.hpp"

#include "Decompressor.hpp"
#include "logger.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace SAP {
    static void* g_OodleLibHandle = nullptr;

#ifdef _WIN32
#include <psapi.h>

  // Hilfsfunktion für das Pattern-Matching
  static uint8_t *findPattern(uint8_t *start, size_t size, const std::vector<uint8_t> &pattern) {
    for (size_t i = 0; i < size - pattern.size(); i++) {
      bool match = true;
      for (size_t j = 0; j < pattern.size(); j++) {
        if (start[i + j] != pattern[j]) {
          match = false;
          break;
        }
      }
      if (match)
        return start + i;
    }
    return nullptr;
  }
#endif

  void OodleLoader::loadOodle(const std::filesystem::path &gamePath) {
    auto start = std::chrono::high_resolution_clock::now();

    std::string libName;
#ifdef _WIN32
    libName = "oo2core_9_win64.dll";
#else
    libName = "liboo2corelinux64.so.9";
#endif
    std::filesystem::path localPath = std::filesystem::current_path() / libName;
    OodleLZ_Decompress decompressFunc = nullptr;
    if (std::filesystem::exists(localPath)) {
      Logger::logInfo("[OODLE]", "Using local Oodle library: {}", localPath.string());
      decompressFunc = loadLib(localPath);
      if (decompressFunc == nullptr) {
        throw std::runtime_error("Oodle library couldn't be loaded");
      }
    }
#ifdef _WIN32
    if (decompressFunc == nullptr) {
      std::filesystem::path coreDllPath = gamePath / "Engine/Binaries/Win64/FactoryGameSteam-Core-Win64-Shipping.dll";
      Logger::logInfo("[OODLE]", "Searching Oodle in Game Core...");

      HMODULE hCore = LoadLibraryExW(coreDllPath.wstring().c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if (hCore) {
        MODULEINFO modInfo;
        if (GetModuleInformation(GetCurrentProcess(), hCore, &modInfo, sizeof(modInfo))) {
          std::vector<uint8_t> pattern = {0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56,
                                          0x41, 0x57, 0x48, 0x81, 0xEC, 0xC8, 0x00, 0x00, 0x00};
          uint8_t *addr = findPattern(static_cast<uint8_t *>(modInfo.lpBaseOfDll), modInfo.SizeOfImage, pattern);
          if (addr) {
            Logger::logInfo("[OODLE]", "Pattern found at offset: 0x{:X}",
                            (uintptr_t) addr - (uintptr_t) modInfo.lpBaseOfDll);
            decompressFunc = reinterpret_cast<OodleLZ_Decompress>(addr);
          }
        }
      } else {
        Logger::logError("[OODLE]", "Core DLL not found at {}", coreDllPath.string());
      }
    }

#endif
    if (decompressFunc == nullptr) {
      throw std::runtime_error(
          "Oodle decompressor could not be located! Put oo2core_9_win64.dll/liboo2corelinux64.so.9 in the app folder.");
    }
    OodleApi::initializePointer(decompressFunc);
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[OODLE]", "Found in {} ms", us.count() / 1000.0);
  }

  OodleLZ_Decompress OodleLoader::loadLib(std::filesystem::path const &path) {
#ifdef _WIN32
    // Windows/Proton Weg
    HMODULE hMod = LoadLibraryW(path.wstring().c_str());
    g_OodleLibHandle = hMod; // Speichern!
    if (!hMod) {
      return nullptr;
    }
    return reinterpret_cast<OodleLZ_Decompress>(GetProcAddress(hMod, "OodleLZ_Decompress"));
#else
    void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_NODELETE);
    g_OodleLibHandle = handle; // Speichern!
    if (!handle) {
      return nullptr;
    }
    return reinterpret_cast<OodleLZ_Decompress>(dlsym(handle, "OodleLZ_Decompress"));
#endif
  }
} // namespace SAP
