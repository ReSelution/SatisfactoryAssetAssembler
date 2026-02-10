//
// Created by oleub on 02.01.26.
//

#include "texture_decoder.hpp"

// Implementierung der Header aktivieren
#define BCDEC_IMPLEMENTATION
#include "bcdec/bcdec.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <cstring>
#include <filesystem>

#include "stb/stb_image_write.h"

namespace SAP {
  std::vector<uint8_t> TextureDecoder::decompressDXT1(uint32_t width, uint32_t height, std::span<const uint8_t> data) {

    std::vector<uint8_t> rgba(width * height * 4);

    for (uint32_t y = 0; y <height ; y+=4) {
      for (uint32_t x = 0; x < width; x+=4) {
        size_t blockIndex = (y/4 * (width / 4) + (x / 4)) * 8;
        if (blockIndex + 8 > data.size()) [[unlikely]] { break; }
        size_t destIndex = (y  * width + x) * 4;
        bcdec_bc1
            (data.data() + blockIndex,
             &rgba[destIndex],
             width *4);
      }
    }
    return rgba;
  }

  std::vector<uint8_t> TextureDecoder::decompressDXT5(uint32_t width, uint32_t height, std::span<const uint8_t> data) {
    std::vector<uint8_t> rgba(width * height * 4);

    for (int y = 0; y < height; y += 4) {
      for (int x = 0; x < width; x += 4) {
        size_t blockIndex = ((y / 4) * (width / 4) + (x / 4)) * 16;
        bcdec_bc3
            (data.data() + blockIndex,
             &rgba[(y * width + x) * 4],
             width * 4);
      }
    }
    return rgba;
  }

  std::vector<uint8_t> TextureDecoder::decompressBC5(uint32_t width, uint32_t height, std::span<const uint8_t> data) {
    std::vector<uint8_t> rgba(width * height * 4);
    for (int y = 0; y < height; y += 4) {
      for (int x = 0; x < width; x += 4) {
        size_t blockIndex = ((y / 4) * (width / 4) + (x / 4)) * 16;
        // bcdec schreibt R und G, B wird oft ignoriert oder muss rekonstruiert werden
        bcdec_bc5(data.data() + blockIndex, &rgba[(y * width + x) * 4], width * 4);

        // Wichtig: Da BC5 oft für Normal Maps genutzt wird,
        // sollte der Blue-Channel im RGBA auf 255 gesetzt werden für die Anzeige
        // (oder man rekonstruiert Z im Shader)
      }
    }
    return rgba;
  }

  // Für G8 (Graustufen, z.B. Roughness-Masken)
  std::vector<uint8_t> TextureDecoder::decodeG8(uint32_t width, uint32_t height, std::span<const uint8_t> data) {
    std::vector<uint8_t> rgba(width * height * 4);
    for (size_t i = 0; i < width * height; ++i) {
      uint8_t g = data[i];
      rgba[i * 4 + 0] = g;   // R
      rgba[i * 4 + 1] = g;   // G
      rgba[i * 4 + 2] = g;   // B
      rgba[i * 4 + 3] = 255; // A
    }
    return rgba;
  }

  std::vector<uint8_t> TextureDecoder::encodePNG(uint32_t width, uint32_t height, std::span<const uint8_t> rgba) {
    int len;
    unsigned char *png_raw_data = stbi_write_png_to_mem(rgba.data(), width * 4, width, height, 4, &len);
    if (!png_raw_data) return {};
    std::vector<uint8_t> pngBuffer(png_raw_data, png_raw_data + len);
    STBIW_FREE(png_raw_data);
    return pngBuffer;
  }

  std::vector<uint8_t> TextureDecoder::cropRGBA
  (const std::vector<uint8_t> &src, uint32_t srcW, uint32_t srcH, uint32_t dstW, uint32_t dstH) {
    std::vector<uint8_t> dst(dstW * dstH * 4);
    for (uint32_t y = 0; y < dstH; ++y) { std::memcpy(&dst[y * dstW * 4], &src[y * srcW * 4], dstW * 4); }
    return dst;
  }
} // SAP
