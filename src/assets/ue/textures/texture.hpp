//
// Created by oleub on 14.12.25.
//

#pragma once

#include "byte_cursor.hpp"
#include "assets/textures/f_texture_2d_mip_map.hpp"
#include "assets/textures/f_virtual_texture_built_data.hpp"
#include "assets/textures/pixel_format.hpp"
#include "core/f_guid.hpp"
#include "core/hash.hpp"

#include "structs/exported_texture_data.hpp"

#include "asset_object.hpp"
#include "assets/textures/FSharedImage.hpp"

namespace SAA {
  class TextureExporter;
  class AssetFileParser;
  enum class TextureFilter : uint32_t;
  enum class TextureGroup : uint32_t;
  enum class TextureCompressionSettings : uint32_t;

  class Texture : public Object {
    friend TextureExporter;

  public:
    enum class ETextureProperty : uint64_t {
        LightingGuid        = "LightingGuid"_h,
        CompressionSettings = "CompressionSettings"_h,
        LODGroup            = "LODGroup"_h,
        Filter              = "Filter"_h,
        SRGB                = "SRGB"_h,
        AssetUserData       = "AssetUserData"_h,
    };

    Texture(std::string_view className = "Texture");

    virtual void decodeTexture(ExportedTextureData &data);

  private:
    void parsePlatformData(ByteCursor &asset, bool parseMipData = true);
    void parseVirtualTexture(ByteCursor &asset);
    EPixelFormat m_format = EPixelFormat::PF_Unknown;
    EPixelFormat m_pixelFormat =  EPixelFormat::PF_Unknown;

  protected:
    void parse() override;

    FGuid m_lightingGuid{};
    TextureCompressionSettings m_compressionSettings{};
    TextureGroup m_LODGroup{};
    TextureFilter m_filter{};
    FVirtualTextureBuiltData m_vtData{};

    bool m_srgb = true;
    int32_t m_sizeX;
    int32_t m_sizeY;
    int32_t m_packedData;
    int32_t m_firstMipToSerialize;
    FSharedImage sharedImage;

    std::vector<FTexture2DMipMap> m_mipmaps;

    void parseCookedPlatformData(ByteCursor &asset, bool parseMipData = true);

    FTexture2DMipMap parseTexture2dMipMap(ByteCursor &cursor, bool parseMipData = true);
  };
}
