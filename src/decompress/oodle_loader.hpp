//
// Created by oleub on 15.01.26.
//

#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>

namespace SAA {
  typedef int64_t (*OodleLZ_Decompress)(const uint8_t *src_buf, int64_t src_len, uint8_t *dst_buf, int64_t dst_len,
                                        int32_t fuzz, int32_t crc, int32_t verbose, uint8_t *dict_buf, int64_t dict_len,
                                        void *unused1, void *unused2, void *unused3, void *unused4,
                                        int32_t threadModule);

  class OodleLoader {
  public:
    static void loadOodle(std::filesystem::path const &gamePath);

  private:
    const std::vector<uint8_t> OODLE_WRAPPER_PATTERN = {0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56,
                                                        0x41, 0x57, 0x48, 0x81, 0xEC, 0xC8, 0x00, 0x00, 0x00};
    static OodleLZ_Decompress loadLib(std::filesystem::path const &path);
  };
} // namespace SAP
