#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <span> // C++20, sonst const uint8_t* und size_t nutzen
#include <vector>

#include "usmap/enums.hpp"

namespace SAP {


  enum class CompressionMethod : uint8_t {
    None = 0,
    Zlib = 1,
    Gzip = 2, //???
    Custom = 3,
    Oodle = 4,
    LZ4 = 5,
    Zstd = 6,
    XB1Zlib = 7,
    Unknown = 8
  };

  // Definiere den Typ für die OodleLZ_Decompress Funktion (vereinfacht)
  using OodleLZ_Decompress_Ptr = long (*)(const void *compressedBytes,
    long compressedSize,
    void *decompressedBytes,
    long decompressedSize,
    int fuzzSafe,
    int checkCRC,
    int verbosity,
    void *unused1,
    long unused2,
    void *unused3,
    void *unused4,
    void *unused5,
    long unused6,
    int DecodeThreadPhase);

  // --- Interne Statische Management-Struktur ---
  namespace OodleApi {

    inline OodleLZ_Decompress_Ptr decompressFunc = nullptr;
    inline void *dllHandle = nullptr;
    inline std::atomic<bool> initialized = false;
    inline std::once_flag initFlag;
    bool initializePointers(void *dllHandle);
  } // namespace OodleApi

  class Decompressor {
  public:
    // Verhindert Instanziierung, da rein statisch
    Decompressor() = delete;
    // Explizite Initialisierungsfunktion, die Sie einmal aufrufen
    static bool initialize(void *dllHandle) {
      // Garantiert, dass InitializePointers nur einmal aufgerufen wird,
      // auch wenn mehrere Threads gleichzeitig versuchen, es zu initialisieren.
      std::call_once(OodleApi::initFlag, [&]() { OodleApi::initialized = OodleApi::initializePointers(dllHandle); });
      return OodleApi::initialized.load();
    }
    static bool decompressUsmapPayload(
      std::span<const uint8_t> compressedData, std::span<uint8_t> uncompressedData, EUsmapCompressionMethod method);
    static bool decompress(
      std::span<const uint8_t> compressedData, std::span<uint8_t> uncompressedData, CompressionMethod method);

  private:
    // Interne Implementierungen
    static bool decompressOodle(std::span<const uint8_t> src, std::span<uint8_t> dst);
    static bool decompressZlib(std::span<const uint8_t> src, std::span<uint8_t> dst);
    static bool decompressLZ4(std::span<const uint8_t> src, std::span<uint8_t> dst);
    static bool decompressGzip(std::span<const uint8_t> src, std::span<uint8_t> dst);
  };

} // namespace SAP