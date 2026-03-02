#include "Decompressor.hpp"

#include <cstdint>
#include <cstring> // für memcpy

#include "logger.hpp"
#include "usmap/enums.hpp"

// HINWEIS: Du musst hier die entsprechenden Libraries inkludieren.
// Für Oodle brauchst du das Oodle SDK oder Wrapper.
// Für Zlib brauchst du <zlib.h>
// #include <zlib.h>
// #include "oodk.h"

namespace SAA {

  void OodleApi::initializePointer(OodleLZ_Decompress func) {
    decompressFunc = func;
    if (func) {
      initialized.store(true);
    } else {
      initialized.store(false);
    }
  }

  bool Decompressor::decompressUsmapPayload(std::span<const uint8_t> compressedData,
                                            std::span<uint8_t> uncompressedData, EUsmapCompressionMethod usmapMethod) {
    CompressionMethod genericMethod{};

    switch (usmapMethod) {
      case EUsmapCompressionMethod::NONE:
        // Wir mappen auf den generischen NONE-Fall
        genericMethod = CompressionMethod::None;
        break;
      case EUsmapCompressionMethod::OODLE:
        // Wir mappen auf den generischen OODLE-Fall
        genericMethod = CompressionMethod::Oodle;
        break;
      case EUsmapCompressionMethod::Z_STANDARD:
        // Wenn Sie Z_STANDARD später implementieren, mappen Sie es hier
        genericMethod = CompressionMethod::Zstd;
        break;
      case EUsmapCompressionMethod::UNKNOWN:
      default:
        Logger::logError("[Decompressor]", "Unbekannte oder nicht unterstützte USMAP-Kompressionsmethode: {}",
                         (int) usmapMethod);
        return false;
    }

    // Rufen Sie die bereits existierende decompress-Funktion mit dem gemappten Enum auf.
    return Decompressor::decompress(compressedData, uncompressedData, genericMethod);
  }

  bool Decompressor::decompress(std::span<const uint8_t> compressedData, std::span<uint8_t> uncompressedData,
                                CompressionMethod method) {

    switch (method) {
      case CompressionMethod::None:
      case CompressionMethod::Unknown: // Optional: Bei Unknown direkt als unkomprimiert behandeln oder Fehler werfen
      {
        if (uncompressedData.size() != compressedData.size()) {
          Logger::logError("[Decompressor]", "Size mismatch for method None. Expected {}, got {}",
                           uncompressedData.size(), compressedData.size());
          return false;
        }
        // Daten direkt kopieren
        std::memcpy(uncompressedData.data(), compressedData.data(), compressedData.size());
        return true;
      }

      case CompressionMethod::Oodle:
        return decompressOodle(compressedData, uncompressedData);

      case CompressionMethod::Zlib:
      case CompressionMethod::XB1Zlib: // XB1Zlib ist oft eine Variante von Zlib
        return decompressZlib(compressedData, uncompressedData);

      case CompressionMethod::LZ4:
        return decompressLZ4(compressedData, uncompressedData);

      case CompressionMethod::Gzip:
        return decompressGzip(compressedData, uncompressedData);

      case CompressionMethod::Custom:
        // Wenn Custom bekannt ist, hier Logik einfügen
        Logger::logError("[Decompressor]", "Compression method Custom ist nicht implementiert.");
        return false;

      case CompressionMethod::Zstd:
        // Muss implementiert werden
        Logger::logError("[Decompressor]", "Compression method Zstd ist nicht implementiert.");
        return false;

      default:
        Logger::logError("[Decompressor]", "Unbekannter oder nicht unterstützter Kompressions-Enum-Wert.");
        return false;
    }
  }

  bool Decompressor::decompressOodle(std::span<const uint8_t> src, std::span<uint8_t> dst) {
    if (!OodleApi::initialized.load() || (OodleApi::decompressFunc == nullptr)) {
      throw std::runtime_error("Decompressor wurde nicht initialisiert oder Oodle fehlt.");
    }

    uint64_t decodedSize = OodleApi::decompressFunc(src.data(), static_cast<long>(src.size()), dst.data(), static_cast<long>(dst.size()), 0, 0,
                                                    0, nullptr, 0, nullptr, nullptr, nullptr, 0,
                                                    0);

    if (decodedSize <= 0) {
      Logger::logError("[Oodle]", "Oodle Dekompression fehlgeschlagen.");
      return false;
    }
    if (decodedSize < dst.size()) {
      Logger::logError("[Oodle]", "Oodle decompression just decompressed {} bytes of the expected {} bytes",
                       decodedSize, dst.size());
    }

    return true;
  }

  bool Decompressor::decompressZlib(std::span<const uint8_t> src, std::span<uint8_t> dst) {
    Logger::logError("[Decompressor]", "Zlib implementation missing.");
    return false;
  }

  // ... Implementierung für LZ4 / Gzip ähnlich ...
  bool Decompressor::decompressLZ4(std::span<const uint8_t> src, std::span<uint8_t> dst) { return false; }
  bool Decompressor::decompressGzip(std::span<const uint8_t> src, std::span<uint8_t> dst) { return false; }

} // namespace SAP
