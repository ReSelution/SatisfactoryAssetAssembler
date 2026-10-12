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
  namespace {
    // FPropertyTagData is only needed while the properties of one object are read. It comes from this buffer of the
    // thread, which starts over when the outermost parseProperties() of the thread returns. The tags that are stored
    // carry no TagData.
    struct TagDataScratch {
      sc::MonotonicResource resource{64 * 1024};
      uint32_t depth = 0;
    };

    thread_local TagDataScratch tagDataScratch;

    struct TagDataScratchScope {
      TagDataScratch &scratch;

      explicit TagDataScratchScope(TagDataScratch &s) : scratch(s) { ++scratch.depth; }
      TagDataScratchScope(const TagDataScratchScope &)            = delete;
      TagDataScratchScope &operator=(const TagDataScratchScope &) = delete;

      ~TagDataScratchScope() {
        if (--scratch.depth == 0) {
          scratch.resource.reset();
        }
      }
    };
  } // namespace

  PropertyHolder::PropertyHolder(const PropertyMap *props) : m_properties(props) {}

  void PropertyHolder::parseProperties(ByteCursor &cursor) {
    TagDataScratchScope scope(tagDataScratch);
    sc::MonotonicResource *storage = &scope.scratch.resource;
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
