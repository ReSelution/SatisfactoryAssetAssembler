//
// Created by oleub on 02.01.26.
//

#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace SAA {
  class TextureDecoder {
  public:
    static std::vector<uint8_t> decompressDXT1(uint32_t width, uint32_t height, std::span<const uint8_t> data);
    static std::vector<uint8_t> decompressDXT5(uint32_t width, uint32_t height, std::span<const uint8_t> data);
    static std::vector<uint8_t> decompressBC5(uint32_t width, uint32_t height, std::span<const uint8_t> data);
    static std::vector<uint8_t> decodeG8(uint32_t width, uint32_t height, std::span<const uint8_t> data);
    static std::vector<uint8_t> encodePNG(uint32_t width, uint32_t height, std::span<const uint8_t> rgba);
    static std::vector<uint8_t> cropRGBA(const std::vector<uint8_t> &src, uint32_t srcW, uint32_t srcH, uint32_t dstW,
                                         uint32_t dstH);
  };
} // namespace SAP
