//
// Created by oleub on 06.02.26.
//

#include "loc_res_parser.hpp"


#include "byte_cursor.hpp"
#include "game_file.hpp"
#include "vfs_entry.hpp"

#include "localization/f_entry.hpp"
#include "sc/hash/hash.hpp"
#include "sc/logger/logger.hpp"
#include "sc/memory/monotonic_resource.hpp"

using LocResLoc = sc::Logger<"SAA", "LOCRES">;

namespace SAA {
  LocResParser::LocResParser(const UnifiedVfsEntry *vfsEntry, bool isNative) :
      isNative(isNative), m_vfsEntry(vfsEntry) {
    magpie = sc::Magpie::get();
  }

  size_t LocResParser::parse() {
    size_t inserted = 0;
    // Lebt nur fuer diesen Aufruf: Datei, Namespaces und die String-Tabelle. Die uebersetzten Texte selbst liegen im
    // Speicher von Magpie.
    sc::MonotonicResource storage;
    auto data = m_vfsEntry->read(&storage);

    auto cursor = data->subspan(0, data->size());
    auto magic  = cursor.read<FGuid>();
    if (magic != LOC_RES_MAGIC) {
      LocResLoc::err("{} found: A: {} B: {} C: {} D: {}", m_vfsEntry->pathStr, magic.A, magic.B, magic.C, magic.D);
      return inserted;
    }
    auto version = cursor.read<ELocResVersion>();

    if (version >= ELocResVersion::Compact) {
      auto localizedStringArrayOffset = cursor.read<int64_t>();
      if (localizedStringArrayOffset != -1) {
        auto pos             = cursor.tell();
        cursor               = localizedStringArrayOffset;
        auto count           = cursor.read<int32_t>();
        localizedStringArray = storage.allocateSpan<FTextLocalizationResourceString>(count);
        if (version >= ELocResVersion::Optimized_CRC32) {
          for (auto i = 0; i < count; i++) {
            // Direkt in den Speicher von Magpie lesen: mt_InsertStored() kopiert den Text nicht mehr, die View muss
            // also bis zum naechsten Magpie::clear() gueltig bleiben.
            auto str = cursor.readFString(magpie); // Dont move inside emplace !!! Breaks under Mingw and Wine
            localizedStringArray[i] = {str, cursor.read<int32_t>()};
          }
        } else {
          for (auto i = 0; i < count; i++) {
            localizedStringArray[i] = {cursor.readFString(magpie), -1};
          }
        }
        cursor = pos;
      }
    }

    if (version >= ELocResVersion::Optimized_CRC32) {
      auto entriesCount = cursor.read<uint32_t>();
      auto nsCount      = cursor.read<uint32_t>();
      magpie->mt_reserve(entriesCount);
      for (uint32_t i = 0; i < nsCount; i++) {
        cursor += sizeof(uint32_t); // skip hash
        auto nsStr  = cursor.readFString(&storage);
        auto nsHash = sc::hash(nsStr);

        const auto keyCount = cursor.read<uint32_t>();
        for (uint32_t j = 0; j < keyCount; j++) {
          cursor += sizeof(uint32_t); // skip hash
          // Only needed for the hash, mt_InsertStored() copies the key itself where it keeps it (DUMP_MAGPIE).
          auto keyStr  = cursor.readFString(&storage);
          auto keyHash = sc::hash(keyStr);
          cursor += sizeof(uint32_t);
          auto stringIndex = cursor.read<uint32_t>();
          if (localizedStringArray.size() > stringIndex) [[likely]] {
            sc::Magpie::mt_InsertStored(sc::MagpieKey{nsHash, keyHash}, localizedStringArray[stringIndex].str, nsStr,
                                        keyStr);
            inserted++;
          }
        }
      }
    }
    magpie->mt_Merge(!isNative);
    return inserted;
  }
} // namespace SAA
