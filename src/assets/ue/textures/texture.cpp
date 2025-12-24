//
// Created by oleub on 14.12.25.
//

#include "texture.hpp"

#include "logger.hpp"
#include "assets/textures/texture_compression_settings.hpp"
#include "assets/textures/texture_filter.hpp"
#include "assets/textures/texture_group.hpp"

namespace SAP {

  Texture::Texture(std::string className) : Object(std::move(className)) {}

  void Texture::parse(size_t &cursor, AssetFileParser *owner) {
    Object::parse(cursor, owner);
    m_lightingGuid = getOrDefault<FGuid>
        (static_cast<uint32_t>(ETextureProperty::LightingGuid), [&] {
          return FGuid::fromUint32(fnv1a(getFullName().c_str()));
        });
    m_compressionSettings = getOrDefaultEnum<TextureCompressionSettings>
        (static_cast<uint32_t>(ETextureProperty::CompressionSettings), [&] {
          return TextureCompressionSettings::TC_Default;
        });
    m_LODGroup = getOrDefaultEnum<TextureGroup>
        (static_cast<uint32_t>(ETextureProperty::LODGroup), [&] { return TextureGroup::World; });
    m_filter = getOrDefaultEnum<TextureFilter>
        (static_cast<uint32_t>(ETextureProperty::Filter), [&] { return TextureFilter::TF_Nearest; });
    m_srgb = getOrDefault<bool>(static_cast<uint32_t>(ETextureProperty::SRGB), [&] { return true; });
  }

  void Texture::parseCookedPlatformData(std::vector<uint8_t> &asset, size_t &cursor, bool parseMipData) {
   auto pixelFormatName =  readFName(asset, cursor);
   Logger::logInfo("[TEXTURE]", "PixelFormate {}", pixelFormatName.text());
    throw std::runtime_error("[TEXTURE]: parseCookedPlatformData not implenteded yeet");
  }
}
