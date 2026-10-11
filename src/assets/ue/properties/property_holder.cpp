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
    m_properties = m_owner->storage()->makePmr<denseMap>();
    m_properties->reserve(10);
    // TODO (messen): Wo sollen die FPropertyTagData (168 B pro Tag) liegen? Vorher kamen sie aus einer Arena des
    // SC::ChaosArenaPool, die hier wieder zurueckgegeben wurde. Im Moment nicht gemessen, nur so gewaehlt:
    // a) Speicher des Parsers (aktuell): kein Zusatzaufwand pro Aufruf und FPropertyTag::TagData bleibt gueltig
    //    (vorher zeigte der Zeiger nach parseProperties() auf zurueckgegebenen Speicher). Der Speicher bleibt aber
    //    bis zum reset() des Parsers belegt.
    // b) Eigene Resource aus dem sc::ArenaPool pro Aufruf (wie frueher): Speicher sofort wieder frei, dafuer pro
    //    geparstem Objekt ein acquire()/release(). TagData in den gespeicherten Tags zeigt danach wieder ins Leere.
    // c) FPropertyTagData gar nicht mehr pro Tag allokieren (siehe Punkt 3 im TODO in property_parser_1017.cpp).
    // Zu vergleichen: Parse-Zeit ueber alle Objekte und Spitzenverbrauch an Speicher (RSS) fuer a) und b).
    sc::MonotonicResource *storage = m_owner->storage();
    auto ver = m_owner->mappings()->pkgVersion.FileVersionUE5;
    switch (ver) {
      case EUE5Version::UE5_3:
        PropertyParser<EUE5Version::UE5_3>::parse(this, cursor, storage);
        break;
      case EUE5Version::UE5_6:
        PropertyParser<EUE5Version::UE5_6>::parse(this, cursor, storage);
        break;
      default:
        break;
    }
  }

  PropertyHolder PropertyHolder::parseFallbackStruct(ByteCursor &cursor) {
    return {};
  }
} 
