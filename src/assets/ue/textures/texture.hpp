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

#include "ue/asset_object.hpp"

namespace SAP {
  class TextureExporter;
  class AssetFileParser;
  enum class TextureFilter : uint32_t;
  enum class TextureGroup : uint32_t;
  enum class TextureCompressionSettings : uint32_t;

  class Texture : public Object {
    friend TextureExporter;

  public:
    enum class ETextureProperty : uint32_t {
      LightingGuid = fnv1a("LightingGuid"),
      CompressionSettings = fnv1a("CompressionSettings"),
      LODGroup = fnv1a("LODGroup"),
      Filter = fnv1a("Filter"),
      SRGB = fnv1a("SRGB"),
      AssetUserData = fnv1a("AssetUserData"),
    };

    Texture(std::string_view className = "Texture");

    void exportAsset() override;
    virtual void decodeTexture(ExportedTextureData &data);

  private:
    void parsePlatformData(ByteCursor &asset, bool parseMipData = true);
    void parseVirtualTexture(ByteCursor &asset);
    EPixelFormat m_format = EPixelFormat::PF_Unknown;
    std::string m_pixelFormat;

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
    std::vector<FTexture2DMipMap> m_mipmaps;

    void parseCookedPlatformData(ByteCursor &asset, bool parseMipData = true);

    FTexture2DMipMap parseTexture2dMipMap(ByteCursor &cursor, bool parseMipData = true);
  };
} // namespace SAP
