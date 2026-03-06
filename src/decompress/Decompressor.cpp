#include "Decompressor.hpp"

#include <cstdint>
#include <cstring>

#include "usmap/enums.hpp"

#include "logger/chaos_logger.hpp"
LOG_ALIAS(DecomLog, "SAA", "Decompressor")

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
        genericMethod = CompressionMethod::None;
        break;
      case EUsmapCompressionMethod::OODLE:
        genericMethod = CompressionMethod::Oodle;
        break;
      case EUsmapCompressionMethod::Z_STANDARD:
        genericMethod = CompressionMethod::Zstd;
        break;
      case EUsmapCompressionMethod::UNKNOWN:
      default:
        DecomLog::err("Unknown compression method: {}",
                      static_cast<int>(usmapMethod));
        return false;
    }

    return decompress(compressedData, uncompressedData, genericMethod);
  }

  bool Decompressor::decompress(std::span<const uint8_t> compressedData, std::span<uint8_t> uncompressedData,
                                CompressionMethod method) {
    switch (method) {
      case CompressionMethod::None:
      case CompressionMethod::Unknown: // Optional: Bei Unknown direkt als unkomprimiert behandeln oder Fehler werfen
      {
        if (uncompressedData.size() != compressedData.size()) {
          DecomLog::err("Size mismatch for method None. Expected {}, got {}",
                        uncompressedData.size(), compressedData.size());
          return false;
        }
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

        DecomLog::err("Compression method Custom not implemented");
        return false;

      case CompressionMethod::Zstd:
        DecomLog::err("Compression method Zstd  not implemented");
        return false;

      default:
        DecomLog::err("Unknown Compression method not implemented");
        return false;
    }
  }

  bool Decompressor::decompressOodle(std::span<const uint8_t> src, std::span<uint8_t> dst) {
    if (!OodleApi::initialized.load() || (OodleApi::decompressFunc == nullptr)) {
      throw std::runtime_error("Decompressor wurde nicht initialisiert oder Oodle fehlt.");
    }

    uint64_t decodedSize = OodleApi::decompressFunc(src.data(), static_cast<long>(src.size()), dst.data(),
                                                    static_cast<long>(dst.size()), 0, 0,
                                                    0, nullptr, 0, nullptr, nullptr, nullptr, 0,
                                                    0);

    if (decodedSize <= 0) {
      DecomLog::err("Oodle Dekompression fehlgeschlagen.");
      return false;
    }
    if (decodedSize < dst.size()) {
      DecomLog::err("Oodle decompression just decompressed {} bytes of the expected {} bytes",
                    decodedSize, dst.size());
    }

    return true;
  }

  bool Decompressor::decompressZlib(std::span<const uint8_t> src, std::span<uint8_t> dst) {
    DecomLog::err("Zlib implementation missing.");
    return false;
  }

  bool Decompressor::decompressLZ4(std::span<const uint8_t> src, std::span<uint8_t> dst) { return false; }
  bool Decompressor::decompressGzip(std::span<const uint8_t> src, std::span<uint8_t> dst) { return false; }
} // namespace SAP
