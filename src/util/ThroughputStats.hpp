#pragma once
#include <atomic>
#include <chrono>

namespace SAA {
  struct ThroughputStats {
    static inline std::atomic<uint64_t> totalDecompressedBytes{0};
    static inline std::atomic<uint64_t> totalCompressedBytes{0};
    static inline std::chrono::high_resolution_clock::time_point startTime;

    static void start() {
      totalDecompressedBytes = 0;
      totalCompressedBytes = 0;
      startTime = std::chrono::high_resolution_clock::now();
    }

    static double getMBPerSecondDecompressed() {
      auto now = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
      if (duration == 0) return 0;

      // (Bytes / 1024 / 1024) / (Microseconds / 1.000.000)
      return (static_cast<double>(totalDecompressedBytes.load()) / (1024.0 * 1024.0)) / (duration / 1000000.0);
    }
    static double getMBPerSecondCompressed() {
      auto now = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
      if (duration == 0) return 0;

      // (Bytes / 1024 / 1024) / (Microseconds / 1.000.000)
      return (static_cast<double>(totalCompressedBytes.load()) / (1024.0 * 1024.0)) / (duration / 1000000.0);
    }
  };

}