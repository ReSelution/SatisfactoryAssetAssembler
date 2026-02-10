//
// Created by oleub on 06.02.26.
//

#include "loc_res_parser.hpp"

#include <utility>

#include "byte_cursor.hpp"
#include "logger.hpp"
#include "PAKReader.hpp"
#include "vfs_entry.hpp"

#include "localization/f_entry.hpp"
#include "localization/f_text_key.hpp"

namespace SAP {
  LocResParser::LocResParser
  (LocManager *manager,  const UnifiedVfsEntry * vfsEntry) : m_manager
                                                                     (manager), m_vfsEntry(std::move(vfsEntry)) {}

  void LocResParser::parse() {
    data = m_vfsEntry->read();
    ByteCursor cursor(data);
    auto magic = cursor.read<FGuid>();
    if (magic != LOC_RES_MAGIC) {
      Logger::logError
          ("[LOCRES]", "{} found: A: {} B: {} C: {} D: {}", m_vfsEntry->path, magic.A, magic.B, magic.C, magic.D);
    }
    auto version = cursor.read<ELocResVersion>();

    if (version >= ELocResVersion::Compact) {
      auto localizedStringArrayOffset = cursor.read<int64_t>();
      if (localizedStringArrayOffset != -1) {
        auto pos = cursor.tell();
        cursor = localizedStringArrayOffset;
        auto count = cursor.read<int32_t>();
        localizedStringArray.reserve(count);
        if (version >= ELocResVersion::Optimized_CRC32) {
          for (auto i = 0; i < count; i++) {
            auto str = cursor.readFString(m_strings); // Dont move inside emplace !!! Breaks under Mingw and Wine
            localizedStringArray.emplace_back(str, cursor.read<int32_t>());
          }
        } else {

          for (auto i = 0; i < count; i++) { localizedStringArray.emplace_back(cursor.readFString(m_strings), -1); }
        }
        cursor = pos;
      }
    }

    if (version >= ELocResVersion::Optimized_CRC32) {
      auto entriesCount = cursor.read<uint32_t>();
      auto namespceCount = cursor.read<uint32_t>();
      for (auto i = 0; i < namespceCount; i++) {
        auto namespce = FTextKey{cursor.read<uint32_t>(), cursor.readFString(m_strings)};
        auto keyCount = cursor.read<uint32_t>();
        auto &keyValue = entries.emplace(namespce, std::unordered_map<FTextKey, FEntry>{}).first->second;
        keyValue.reserve(keyCount);
        for (auto j = 0; j < keyCount; j++) {
          auto key = FTextKey{cursor.read<uint32_t>(), cursor.readFString(m_strings),};
          FEntry entry{"", cursor.read<uint32_t>()};
          auto stringIndex = cursor.read<uint32_t>();
          if (localizedStringArray.size() > stringIndex) {
            entry.localizedString = localizedStringArray[stringIndex].str;
          }
          keyValue.emplace(key, entry);
        }
      }
    }
  }
} // SAP
