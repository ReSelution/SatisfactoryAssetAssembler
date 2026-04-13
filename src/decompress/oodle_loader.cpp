//
// Created by oleub on 15.01.26.
//

#include "oodle_loader.hpp"

#include <fstream>

#include "Decompressor.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "logger/chaos_logger.hpp"
LOG_ALIAS(OodleLog, "SAA", "OODLE")

namespace SAA {
  static void *g_OodleLibHandle = nullptr;

#ifdef _WIN32
#include <psapi.h>

  void dumpModuleMemory(HMODULE hMod, const std::string &filename) {
    OodleLog::info("Dumping...");

    if (!hMod) return;

    MODULEINFO modInfo;
    if (GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo))) {
      uint8_t *baseAddr = static_cast<uint8_t *>(modInfo.lpBaseOfDll);
      size_t imageSize = modInfo.SizeOfImage;

      std::ofstream dumpFile(filename, std::ios::binary);
      if (dumpFile.is_open()) {
        dumpFile.write(reinterpret_cast<const char *>(baseAddr), imageSize);
        dumpFile.close();
        OodleLog::info("Module dumped to: {} (Size: {} bytes)", filename, imageSize);
      } else {
        OodleLog::err("Failed to create dump file!");
      }
    }
  }

  static uint8_t *findPattern(uint8_t *start, size_t size, const std::vector<uint8_t> &pattern) {
    if (pattern.empty() || size < pattern.size()) return nullptr;

    // 1. Finde das erste Byte im Pattern, das KEINE Wildcard ist
    size_t anchorIdx = 0;
    while (anchorIdx < pattern.size() && pattern[anchorIdx] == 0xCC) {
      anchorIdx++;
    }

    // Falls das ganze Pattern nur aus Wildcards besteht (unwahrscheinlich)
    if (anchorIdx == pattern.size()) return start;

    uint8_t firstFixedByte = pattern[anchorIdx];
    uint8_t *scanPos = start + anchorIdx;
    uint8_t *end = start + size - pattern.size() + anchorIdx;

    while (scanPos <= end) {
      // Suche blitzschnell nach dem ersten fixen Byte
      uint8_t *found = (uint8_t *) memchr(scanPos, firstFixedByte, end - scanPos + 1);
      if (!found) break;

      // Wir haben den Anker gefunden, jetzt prüfen wir den Rest des Patterns
      uint8_t *potentialMatch = found - anchorIdx;
      bool match = true;

      for (size_t j = 0; j < pattern.size(); j++) {
        // Überspringe den Check für das Byte, das memchr schon gefunden hat
        if (j == anchorIdx) continue;

        if (pattern[j] != 0xCC && potentialMatch[j] != pattern[j]) {
          match = false;
          break;
        }
      }

      if (match) {
        return potentialMatch;
      }

      // Kein Match? Weitersuchen nach dem nächsten Anker-Byte
      scanPos = found + 1;
    }

    return nullptr;
  }
#endif

  void OodleLoader::loadOodle(const std::filesystem::path &gamePath) {
    auto t = OodleLog::time("Found in {}");

    std::string libName;
#ifdef _WIN32
    libName = "oo2core_9_win64.dll";
#else
    libName = "liboo2corelinux64.so.9";
#endif
    std::filesystem::path localPath = std::filesystem::current_path() / libName;
    OodleLZ_Decompress decompressFunc = nullptr;
    if (std::filesystem::exists(localPath)) {
      OodleLog::info("Using local Oodle library: {}", localPath.string());
      decompressFunc = loadLib(localPath);
      if (decompressFunc == nullptr) {
        throw std::runtime_error("Oodle library couldn't be loaded");
      }
    }
#ifdef _WIN32
    if (decompressFunc == nullptr) {
      std::filesystem::path coreDllPath = gamePath / "Engine/Binaries/Win64/FactoryGameSteam-Core-Win64-Shipping.dll";
      OodleLog::info("Searching Oodle in Game Core...");
      std::filesystem::path binPath = coreDllPath.parent_path();
      SetDllDirectoryW(binPath.wstring().c_str());

      HMODULE hCore = LoadLibraryExW(
        coreDllPath.wstring().c_str(),
        NULL,
        LOAD_WITH_ALTERED_SEARCH_PATH
      );
      if (hCore) {
        MODULEINFO modInfo;
        if (GetModuleInformation(GetCurrentProcess(), hCore, &modInfo, sizeof(modInfo))) {
          std::vector<uint8_t> pattern = {
            0x40, 0x55, // PUSH RBP
            0x53, // PUSH RBX
            0x56, // PUSH RSI
            0x57, // PUSH RDI
            0x41, 0x54, // PUSH R12
            0x41, 0x55, // PUSH R13
            0x41, 0x56, // PUSH R14
            0x41, 0x57, // PUSH R15
            0x48, 0x81, 0xEC, 0xCC, 0xCC, 0x00, 0x00, // SUB RSP, [Wildcard]
            0x48, 0x8D, 0x6C, 0x24, 0x60 // LEA RBP, [RSP + 60h]  <-- DER ANKER
          };
          //dumpModuleMemory(hCore, "factory_core_memory_dump.bin");
          auto base = static_cast<uint8_t *>(modInfo.lpBaseOfDll);
          uint8_t *addr = findPattern(base, modInfo.SizeOfImage, pattern);
          if (addr) {
            OodleLog::info("Pattern found at offset: 0x{:X}",
                           (uintptr_t) addr - (uintptr_t) modInfo.lpBaseOfDll);

            uint8_t *realStart = addr;
            while (realStart > base && *(realStart - 1) == 0x3E) {
              realStart--;
            }
            OodleLog::info("Found at: 0x{:X}, Function Start: 0x{:X}",
                           (uintptr_t) addr, (uintptr_t) realStart);

            decompressFunc = reinterpret_cast<OodleLZ_Decompress>(realStart);
          }
        }
      } else {
        OodleLog::info("LoadLibraryExW failed! Error: {}", GetLastError());
      }
    }

#endif
    if (decompressFunc == nullptr) {
      throw std::runtime_error(
        "Oodle decompressor could not be located! Put oo2core_9_win64.dll/liboo2corelinux64.so.9 in the app folder.");
    }
    OodleApi::initializePointer(decompressFunc);
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
      typedef int64_t (*Oodle_SetUsageWarnings)(uint32_t  state);

    auto func = reinterpret_cast<Oodle_SetUsageWarnings>(dlsym(handle, "Oodle_SetUsageWarnings"));
    func(1);


    return reinterpret_cast<OodleLZ_Decompress>(dlsym(handle, "OodleLZ_Decompress"));
#endif
  }
} 
