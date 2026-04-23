//
// Created by oleub on 14.12.25.
//

#include "texture.hpp"

#include "texture/texture_exporter.hpp"
#include "assets/textures/f_opt_texture_platfrom_data.hpp"
#include "assets/textures/pixel_format.hpp"
#include "uasset_parser.hpp"

namespace SAA {
  Texture::Texture(std::string_view className) : Object(className) {}

  void Texture::parse() {
    Object::parse();
    // m_lightingGuid = getOrDefault<FGuid>
    //     (static_cast<uint32_t>(ETextureProperty::LightingGuid), [&] {
    //       return FGuid::fromUint32(fnv1a(getFullName().c_str()));
    //     });
    // m_compressionSettings = getOrDefaultEnum<TextureCompressionSettings>
    //     (static_cast<uint32_t>(ETextureProperty::CompressionSettings), [&] {
    //       return TextureCompressionSettings::TC_Default;
    //     });
    // m_LODGroup = getOrDefaultEnum<TextureGroup>
    //     (static_cast<uint32_t>(ETextureProperty::LODGroup), [&] { return TextureGroup::World; });
    // m_filter = getOrDefaultEnum<TextureFilter>
    //     (static_cast<uint32_t>(ETextureProperty::Filter), [&] { return TextureFilter::TF_Nearest; });
    // m_srgb = getOrDefault<bool>(static_cast<uint32_t>(ETextureProperty::SRGB), [&] { return true; });
    m_cursor += 2; // Skip FStripDataFlags
  }


  void Texture::decodeTexture(ExportedTextureData &data) { TextureExporter::decodeTexture(data, this); }

  void Texture::parseCookedPlatformData(ByteCursor &cursor, bool parseMipData) {
    auto pixelFormatName = cursor.readFName();
    while (!pixelFormatName.isNone()) {
      auto pixelFormat = SC::hash<EPixelFormat>(pixelFormatName.text());
      auto skipOffset = +cursor.tell() + cursor.absOffset() + cursor.read<uint64_t>();

      if (m_format == EPixelFormat::PF_Unknown) {
        parsePlatformData(cursor, parseMipData);
        m_format = SC::hash<EPixelFormat>(pixelFormatName.text());
      }

      pixelFormatName = cursor.readFName();
    }
  }

  FTexture2DMipMap Texture::parseTexture2dMipMap(ByteCursor &cursor, bool parseMipData) {
    FTexture2DMipMap texture{};

    bool cooked = reinterpret_cast<UAssetParser *>(m_owner)->getSummary().PackageFlags & PKG_FILTER_EDITOR_ONLY;

    if (parseMipData) {
      texture.data = m_owner->parseFByteBulkData(cursor);
    }

    texture.sizeX = cursor.read<int32_t>();
    texture.sizeY = cursor.read<int32_t>();
    texture.sizeZ = cursor.read<int32_t>();
    return texture;
  }

  void Texture::parsePlatformData(ByteCursor &asset, bool parseMipData) {
    constexpr uint32_t BITMASK_CUBE_MAP = 1u << 31;
    constexpr uint32_t BITMASK_HAS_OPT_DATA = 1u << 30;
    constexpr uint32_t BITMASK_HAS_CPU_COPY = 1u << 29;
    constexpr uint32_t BITMASK_NUM_SLICES = BITMASK_HAS_OPT_DATA - 1u;

    if ((reinterpret_cast<UAssetParser *>(m_owner)->getSummary().PackageFlags & PKG_FILTER_EDITOR_ONLY)) {
      asset.skip(16);
    }

    m_sizeX = asset.read<int32_t>();
    m_sizeY = asset.read<int32_t>();
    m_packedData = asset.read<int32_t>();

    m_pixelFormat = SC::hash<EPixelFormat>(asset.readFString());

    if (m_packedData & BITMASK_HAS_OPT_DATA) {
      asset.read<FOptTexturePlatformData>();
    }

    if (m_packedData & BITMASK_HAS_CPU_COPY) {
      sharedImage.sizeX = asset.read<int32_t>();
      sharedImage.sizeY = asset.read<int32_t>();
      sharedImage.sizeZ = asset.read<int32_t>();
      sharedImage.format = asset.read<EPixelFormatByte>();
      sharedImage.gammaSpace = asset.read<uint8_t>();
      auto len = asset.read<int64_t>();
      auto bulk = storage()->make<FByteBulkData>();
      bulk->bulkCursor =  asset.subspan(asset.tell(), static_cast<int32_t>(len));
      asset += static_cast<int32_t>(len);
      sharedImage.mip = FTexture2DMipMap{bulk, sharedImage.sizeX, sharedImage.sizeY, sharedImage.sizeZ};
    }

    m_firstMipToSerialize = asset.read<int32_t>();

    auto mipCount = asset.read<uint32_t>();
    m_mipmaps.reserve(mipCount);
    for (uint32_t mip = 0; mip < mipCount; mip++) {
      m_mipmaps.emplace_back(parseTexture2dMipMap(asset, parseMipData));
    }

    if (asset.readBool()) {
      parseVirtualTexture(asset);
    }
    if (!m_mipmaps.empty()) {
      m_sizeX = m_mipmaps[0].sizeX;
      m_sizeY = m_mipmaps[0].sizeY;
    }
  }

  void Texture::parseVirtualTexture(ByteCursor &asset) {
    auto cooked = asset.readBool();
    m_vtData.numLayers = asset.read<uint32_t>();
    m_vtData.widthInBlocks = asset.read<uint32_t>();
    m_vtData.heightInBlocks = asset.read<uint32_t>();
    m_vtData.tileSize = asset.read<uint32_t>();
    m_vtData.tileBorderSize = asset.read<uint32_t>();
    m_vtData.tileDataOffsetPerLayer = asset.readArray<uint32_t>();

    m_vtData.numMips = asset.read<uint32_t>();
    m_vtData.width = asset.read<uint32_t>();
    m_vtData.height = asset.read<uint32_t>();
    m_vtData.chunkIndexPerMip = asset.readArray<uint32_t>();
    m_vtData.baseOffsetPerMip = asset.readArray<uint32_t>();

    auto tileCount = asset.read<uint32_t>();
    m_vtData.tileOffsetData.resize(tileCount);
    for (uint32_t i = 0; i < tileCount; i++) {
      m_vtData.tileOffsetData.emplace_back(FVirtualTextureTileOffsetData{
          .width = asset.read<uint32_t>(),
          .height = asset.read<uint32_t>(),
          .maxAddress = asset.read<uint32_t>(),
          .addresses = asset.readArray<uint32_t>(),
          .offsets = asset.readArray<uint32_t>(),
      });
    }

    m_vtData.tileIndexPerChunk = asset.readArray<uint32_t>();
    m_vtData.tileIndexPerMip = asset.readArray<uint32_t>();
    m_vtData.tileOffsetInChunk = asset.readArray<uint32_t>();

    m_vtData.layerTypes.resize(m_vtData.numLayers);
    for (auto i = 0; i < m_vtData.numLayers; i++) {
      m_vtData.layerTypes.emplace_back(SC::hash<EPixelFormat>(asset.readFString()));
    }

    m_vtData.layerFallbackColors = asset.readArray<FLinearColor>(m_vtData.numLayers);

    auto chunkCount = asset.read<uint32_t>();
    m_vtData.chunks.reserve(chunkCount);
    for (int i = 0; i < chunkCount; i++) {
      FVirtualTextureDataChunk chunk;
      asset += FSHA_HASH_SIZE;
      chunk.byteSize = asset.read<uint32_t>();
      chunk.codecPayloadSiz = asset.read<uint32_t>();

      chunk.codecType.reserve(m_vtData.numLayers);
      chunk.codecPayloadOffset.reserve(m_vtData.numLayers);
      for (int l = 0; l < m_vtData.numLayers; l++) {
        chunk.codecType.emplace_back(asset.read<EVirtualTextureCodec>());
        chunk.codecPayloadOffset.emplace_back(asset.read<uint32_t>());
      }
      chunk.bulkData = m_owner->parseFByteBulkData(asset);
      m_vtData.chunks.emplace_back(std::move(chunk));
    }
  }
} 
