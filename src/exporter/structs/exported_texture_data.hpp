//
// Created by oleub on 13.01.26.
//

#pragma once
#include <string>
#include <vector>
#include "../flags/TextureExportFlags.hpp"

namespace SAA {
  enum class EImageMimeType : uint32_t { None, PNG = static_cast<uint32_t>(TextureExportFlags::PNG), RawRGBA };

  struct ExportedTextureData {
    std::vector<uint8_t> data;
    uint32_t width;
    uint32_t height;
    EImageMimeType mimeType;
    std::string name;
  };
} // namespace SAP
