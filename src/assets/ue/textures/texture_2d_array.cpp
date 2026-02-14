//
// Created by oleub on 16.01.26.
//

#include "texture_2d_array.hpp"

#include "texture/texture_exporter.hpp"

namespace SAP {
  void Texture2DArray::parse() {
    Texture::parse();
    m_cursor += sizeof(FStripDataFlags);
    if (m_cursor.readBool()) {
      parseCookedPlatformData(m_cursor, true);
    }
  }

  void Texture2DArray::decodeTexture(ExportedTextureData &data) { TextureExporter::decodeTexture(data, this); }

} // namespace SAP
