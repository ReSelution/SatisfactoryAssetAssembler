//
// Created by oleub on 25.12.25.
//

#pragma once
#include "exporter.hpp"

#include "structs/exported_texture_data.hpp"

#include "ue/textures/texture.hpp"
#include "ue/textures/texture_2d_array.hpp"

namespace SAA {
  class TextureExporter : public Exporter {
  public:
    TextureExporter() : Exporter() {};

    static void exportAsset(Texture *texture);
    static ExportedTextureData encodeAsset(Texture *texture, EImageMimeType type);

    static void decodeTexture(ExportedTextureData &data, Texture *texture);
    static void decodeTexture(ExportedTextureData &data, Texture2DArray *texture);

  private:
    static void decodeBytes(ExportedTextureData &data, const FTexture2DMipMap &mip, Texture *texture);
    static void encodeBytes(ExportedTextureData &data);
    static void saveTextures(std::string_view path, ExportedTextureData &data);
  };
} 
