//
// Created by oleub on 09.02.26.
//

#include "property_holder.hpp"

#include "AssetFileParser.hpp"
#include "core/hash.hpp"
#include "f_property_tag.hpp"
#include "PropertyTracker.hpp"
#include "property_parser.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAA {
  PropertyHolder::PropertyHolder(std::string_view className) : m_className(className) {
  }

  PropertyHolder::PropertyHolder(denseMap* props) : m_properties(props) {
    m_properties->reserve(10);
  }

  void PropertyHolder::parseProperties(ByteCursor &cursor) {
    ZoneScopedN("ParseProperties");
    m_properties = m_owner->storage()->make<denseMap> (m_owner->storage()->resource());
    m_properties->reserve(10);
    auto storage = SC::ChaosArenaPool::acquire();
    auto ver = m_owner->mappings()->pkgVersion.FileVersionUE5;
    switch (ver) {
      case EUE5Version::UE5_3:
        PropertyParser<EUE5Version::UE5_3>::parse(this, cursor, storage.get());
        break;
      case EUE5Version::UE5_6:
        PropertyParser<EUE5Version::UE5_6>::parse(this, cursor, storage.get());
        break;
      default:
        break;
    }
    SC::ChaosArenaPool::release(storage);
  }

  PropertyHolder PropertyHolder::parseFallbackStruct(ByteCursor &cursor) {
    return {};
  }
} 
