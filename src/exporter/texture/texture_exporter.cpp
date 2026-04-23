//
// Created by oleub on 25.12.25.
//

#include "texture_exporter.hpp"
#include <fstream>
#include "texture_decoder.hpp"

#include "logger/chaos_logger.hpp"
LOG_ALIAS(TexExLog, "SAA", "TextureExporter")

namespace SAA {
  void TextureExporter::exportAsset(Texture *texture) {
    ExportedTextureData data{.mimeType = static_cast<EImageMimeType>(options().textureExportTypes.load())};
    texture->decodeTexture(data);
    data.mimeType = static_cast<EImageMimeType>(options().textureExportTypes.load());
    encodeBytes(data);
    saveTextures(texture->path(), data);
  }

  ExportedTextureData TextureExporter::encodeAsset(Texture *texture, EImageMimeType type) {
    ExportedTextureData data{.mimeType = type, .name = std::string(texture->objName())};
    texture->decodeTexture(data);
    encodeBytes(data);
    return data;
  }

  void TextureExporter::decodeTexture(ExportedTextureData &data, Texture *texture) {
    FTexture2DMipMap mip{};
    for (auto &m_mipmap: texture->m_mipmaps) {
      if (m_mipmap.data->bulkCursor.size() != 0) {
        mip = m_mipmap;
        break;
      }
    }
    decodeBytes(data, mip, texture);
  }

  void TextureExporter::decodeTexture(ExportedTextureData &data, Texture2DArray *texture) {
    FTexture2DMipMap mip{};
    for (auto &m_mipmap: texture->m_mipmaps) {
      if (m_mipmap.data->bulkCursor.size() != 0) {
        mip = m_mipmap;
        break;
      }
    }
    decodeBytes(data, mip, texture);
  }

  void TextureExporter::decodeBytes(ExportedTextureData &data, const FTexture2DMipMap &mip, Texture *texture) {
    data.width = mip.sizeX;
    data.height = mip.sizeY;
    switch (texture->m_format) {
      case EPixelFormat::PF_DXT5: {
        data.data = TextureDecoder::decompressDXT5(data.width, data.height, mip.data->bulkCursor.buffer());
        break;
      }
      case EPixelFormat::PF_BC5: {
        data.data = TextureDecoder::decompressBC5(data.width, data.height, mip.data->bulkCursor.buffer());
        break;
      }
      case EPixelFormat::PF_G8: {
        data.data = TextureDecoder::decodeG8(data.width, data.height, mip.data->bulkCursor.buffer());
        break;
      }
      case EPixelFormat::PF_DXT1: {
        data.data = TextureDecoder::decompressDXT1(data.width, data.height, mip.data->bulkCursor.buffer());
        break;
      }
      default:
       TexExLog::err("Unknown texture format {}", to_string(texture->m_format));
    }
  }

  void TextureExporter::encodeBytes(ExportedTextureData &data) {
    switch (data.mimeType) {
      case EImageMimeType::PNG:
        data.data = TextureDecoder::encodePNG(data.width, data.height, data.data);
        break;
      default:
        break;
    }
  }

  void TextureExporter::saveTextures(std::string_view path, ExportedTextureData &data) {
    std::filesystem::path textureFile(path);

    switch (data.mimeType) {
      case EImageMimeType::PNG:
        textureFile.replace_extension(".png");
        break;
      default:
        break;
    }
    std::filesystem::path texturedir = textureFile.parent_path();
    if (!texturedir.empty() && !std::filesystem::exists(texturedir)) {
      std::filesystem::create_directories(texturedir);
    }
    std::ofstream file(textureFile, std::ios::out | std::ios::binary);

    if (file.is_open()) {
      // Cast von uint8_t* nach const char* für den Stream
      file.write(reinterpret_cast<const char *>(data.data.data()), data.data.size());
      file.close();
    } else [[unlikely]] {
     TexExLog::err("Failed to open file for writing: {}", path);
    }
  }
} 
