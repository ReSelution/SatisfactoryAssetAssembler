#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <span> // C++20, sonst const uint8_t* und size_t nutzen
#include <vector>

#include "oodle_loader.hpp"

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

 inline  CompressionMethod stringToCompressionMethod(const std::string &name) {
    if (name == "Zlib") { return CompressionMethod::Zlib; }
    if (name == "Gzip") { return CompressionMethod::Gzip; }
    if (name == "Custom") { return CompressionMethod::Custom; }
    if (name == "Oodle") { return CompressionMethod::Oodle; }
    if (name == "LZ4") { return CompressionMethod::LZ4; }
    if (name == "Zstd") { return CompressionMethod::Zstd; }
    if (name == "XB1Zlib") { return CompressionMethod::XB1Zlib; }
    // Default-Fall
    return CompressionMethod::Unknown;
  }

  // --- Interne Statische Management-Struktur ---
  namespace OodleApi {
    inline OodleLZ_Decompress decompressFunc = nullptr;
    inline std::atomic<bool> initialized = false;
    void initializePointer(OodleLZ_Decompress func);
  } // namespace OodleApi

  class Decompressor {
  public:
    Decompressor() = delete;

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