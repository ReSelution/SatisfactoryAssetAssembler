//
// Created by oleub on 12.12.25.
//

#include "texture_2d.hpp"

#include "uasset_parser.hpp"

namespace SAP {
  Texture2D::Texture2D() : Texture("Texture2D") {}
  void Texture2D::parse(size_t &cursor, AssetFileParser *owner) {
    Texture::parse(cursor, owner);
    m_importedSize = getOrDefault<IntPoint>(cursor, [&] { return m_importedSize; });

    cursor +=2; // Skip FStripDataFlags(2bytes)
    auto parser = reinterpret_cast<UAssetParser *>(owner);
    auto asset = parser->getAsset();

    bool cooked = readPrimitive<uint32_t>(asset, cursor) == 1;

    if (cooked) {
      bool serializeMipData = readPrimitive<uint8_t>(asset, cursor) == 1;
      parseCookedPlatformData(asset, cursor, serializeMipData);
    }

  }
}
