//
// Created by oleub on 09.02.26.
//

#include "property_holder.hpp"

#include "AssetFileParser.hpp"
#include "core/f_guid.hpp"
#include "core/hash.hpp"
#include "f_property_tag.hpp"
#include "logger.hpp"
#include "PropertyTracker.hpp"
#include "property_parser.hpp"
#include "thread_arena.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAA {
  PropertyHolder::PropertyHolder(std::string_view className): m_className(className) {}

  PropertyHolder::PropertyHolder(std::pmr::memory_resource *resource): m_properties(resource) {}

  void PropertyHolder::parseProperties(ByteCursor &cursor) {
    m_properties = std::pmr::unordered_map<uint32_t, FPropertyTag>(m_owner->storage()->memoryResource());
    m_properties.reserve(10);
    auto ver = m_owner->mappings()->pkgVersion.FileVersionUE5;
    switch (ver){
      case EUE5Version::UE5_3:
        PropertyParser<EUE5Version::UE5_3>::parse(this, cursor);
        break;
      case EUE5Version::UE5_6:
        PropertyParser<EUE5Version::UE5_6>::parse(this, cursor);
        break;
        default:
        break;
    }

  }


  void PropertyHolder::releaseResources() { ThreadArena::reset(); }

  PropertyHolder PropertyHolder::parseFallbackStruct(ByteCursor &cursor) {
    return {};
  }
} // namespace SAP
