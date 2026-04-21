//
// Created by oleub on 06.02.26.
//

#include "loc_res_parser.hpp"

#include <utility>

#include "PAKReader.hpp"
#include "byte_cursor.hpp"
#include "logger/chaos_logger.hpp"
#include "vfs_entry.hpp"

#include "localization/f_entry.hpp"
#include "localization/f_text_key.hpp"
#include "memory/chaos_bump_arena.hpp"

LOG_ALIAS(LocResLoc, "SAA", "LOCRES");

namespace SAA {
  LocResParser::LocResParser(const UnifiedVfsEntry *vfsEntry, bool isNative)
    : isNative(isNative), m_vfsEntry(vfsEntry) {
    magpie = SC::Magpie::get();
  }

  void LocResParser::parse() {
    m_storage = std::make_unique<SC::ChaosBumpArena>();
    auto data = m_vfsEntry->read(m_storage.get());

    auto cursor = data->subspan(0, data->size());
    auto magic = cursor.read<FGuid>();
    if (magic != LOC_RES_MAGIC) {
      LocResLoc::err("{} found: A: {} B: {} C: {} D: {}", m_vfsEntry->path,
                     magic.A, magic.B, magic.C, magic.D);
    }
    auto version = cursor.read<ELocResVersion>();

    if (version >= ELocResVersion::Compact) {
      auto localizedStringArrayOffset = cursor.read<int64_t>();
      if (localizedStringArrayOffset != -1) {
        auto pos = cursor.tell();
        cursor = localizedStringArrayOffset;
        auto count = cursor.read<int32_t>();
        localizedStringArray =
            m_storage->allocateSpan<FTextLocalizationResourceString>(count);
        if (version >= ELocResVersion::Optimized_CRC32) {
          for (auto i = 0; i < count; i++) {
            auto str = cursor.readFString(m_storage.get()); // Dont move inside emplace !!! Breaks under Mingw and Wine
            localizedStringArray[i] = {str, cursor.read<int32_t>()};
          }
        } else {
          for (auto i = 0; i < count; i++) {
            localizedStringArray[i] = {cursor.readFString(m_storage.get()), -1};
          }
        }
        cursor = pos;
      }
    }

    if (version >= ELocResVersion::Optimized_CRC32) {
      auto entriesCount = cursor.read<uint32_t>();
      auto nsCount = cursor.read<uint32_t>();
      magpie->mt_reserve(entriesCount);
      for (auto i = 0; i < nsCount; i++) {
        cursor += sizeof(uint32_t); // skip hash
        auto nsStr = cursor.readFString(m_storage.get());
        auto nsHash = SC::hash(nsStr);

        const auto keyCount = cursor.read<uint32_t>();
        for (auto j = 0; j < keyCount; j++) {
          cursor += sizeof(uint32_t); // skip hash
          auto keyStr = cursor.readFString(m_storage.get());
          auto keyHash = SC::hash(keyStr);
          cursor += sizeof(uint32_t);
          auto stringIndex = cursor.read<uint32_t>();
          if (localizedStringArray.size() > stringIndex) [[likely]] {
            magpie->mt_Insert(SC::MagpieKey{nsHash, keyHash}, localizedStringArray[stringIndex].str, nsStr, keyStr);
          }
        }
      }
    }
    magpie->mt_Merge(!isNative);
  }
} // namespace SAA
