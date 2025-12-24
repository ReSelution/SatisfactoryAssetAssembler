//
// Created by oleub on 14.12.25.
//

#pragma once

#include "core/f_guid.hpp"
#include "core/hash.hpp"
#include "ue/asset_object.hpp"


namespace SAP {
  class AssetFileParser;
  enum class TextureFilter : uint8_t;
  enum class TextureGroup : uint8_t;
  enum class TextureCompressionSettings : uint8_t;

  class Texture : public Object {


  protected:
    FGuid m_lightingGuid{};
    TextureCompressionSettings m_compressionSettings{};
    TextureGroup m_LODGroup{};
    TextureFilter m_filter{};
    bool m_srgb = true;

    void parseCookedPlatformData(std::vector<uint8_t> &asset, size_t &cursor, bool parseMipData = true );

  public:
    enum class ETextureProperty : uint32_t {
      LightingGuid        = fnv1a("LightingGuid"),
      CompressionSettings = fnv1a("CompressionSettings"),
      LODGroup            = fnv1a("LODGroup"),
      Filter              = fnv1a("Filter"),
      SRGB                = fnv1a("SRGB"),
      AssetUserData       = fnv1a("AssetUserData"),
    };

    Texture(std::string className = "Texture");

    void parse(size_t &cursor, AssetFileParser *owner) override;
  };
}
