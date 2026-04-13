//
// Created by oleub on 12.12.25.
//

#include "texture_2d.hpp"

#include <utility>

#include "uasset_parser.hpp"

namespace SAA {
  Texture2D::Texture2D(std::string_view className) : Texture(className) {}
  void Texture2D::parse() {
    Texture::parse();
    // m_importedSize = getOrDefault<IntPoint>(m_cursor.tell(), [&] { return m_importedSize; });

    m_cursor += 2; // Skip FStripDataFlags(2bytes)

    if (m_cursor.readBool()) {
      bool serializeMipData = m_cursor.readBool();
      parseCookedPlatformData(m_cursor, serializeMipData);
    }
  }
} 
