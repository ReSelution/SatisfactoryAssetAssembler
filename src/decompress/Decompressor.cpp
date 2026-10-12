#include "Decompressor.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include "sc/logger/logger.hpp"
#include "usmap/enums.hpp"


namespace SAA {
  using DecomLog = sc::Logger<"SAA", "Decompressor">;
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
        DecomLog::err("Unknown compression method: {}", static_cast<int>(usmapMethod));
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
          DecomLog::err("Size mismatch for method None. Expected {}, got {}", uncompressedData.size(),
                        compressedData.size());
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

    // Without its own scratch memory Oodle allocates and frees it on every call. Under Wine that goes through the
    // process heap and a global lock, the crawl then barely scales with the threads.
    // One buffer per thread, sized once.
    thread_local std::vector<uint8_t> scratch(OodleApi::scratchSize);
    // Too small a buffer makes the decode fail, so a block above the size it is good for decodes without it.
    const bool useScratch = dst.size() <= OodleApi::scratchMaxRawLen;

    int64_t decodedSize =
        OodleApi::decompressFunc(src.data(), static_cast<long>(src.size()), dst.data(), static_cast<long>(dst.size()),
                                 1, 0, 0, nullptr, 0, nullptr, nullptr, useScratch ? scratch.data() : nullptr,
                                 reinterpret_cast<void *>(useScratch ? scratch.size() : size_t{0}), 0);
    if (decodedSize <= 0 && useScratch) {
      // In case the buffer was too small after all: once more with the memory Oodle allocates itself.
      decodedSize = OodleApi::decompressFunc(src.data(), static_cast<long>(src.size()), dst.data(),
                                             static_cast<long>(dst.size()), 1, 0, 0, nullptr, 0, nullptr, nullptr,
                                             nullptr, nullptr, 0);
    }

    if (decodedSize <= 0) {
      DecomLog::err("Oodle Dekompression fehlgeschlagen.");
      return false;
    }
    if (static_cast<size_t>(decodedSize) != dst.size()) {
      DecomLog::err("Oodle decompression decompressed {} bytes of the expected {} bytes", decodedSize, dst.size());
    }

    return true;
  }

  bool Decompressor::decompressZlib(std::span<const uint8_t> src, std::span<uint8_t> dst) {
    DecomLog::err("Zlib implementation missing.");
    return false;
  }

  bool Decompressor::decompressLZ4(std::span<const uint8_t> src, std::span<uint8_t> dst) { return false; }
  bool Decompressor::decompressGzip(std::span<const uint8_t> src, std::span<uint8_t> dst) { return false; }
} // namespace SAA
