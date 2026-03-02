//
// Created by oleub on 06.02.26.
//

#pragma once

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "heap_storage.hpp"

#include "core/f_guid.hpp"

#include "localization/f_entry.hpp"
#include "localization/f_text_key.hpp"
#include "localization/f_text_localization_resource_string.hpp"

namespace SAA {
  class UnifiedVfsEntry;
  class LocManager;

  enum class ELocResVersion : uint8_t {
    /** Legacy format file - will be missing the magic number. */
    Legacy = 0,
    /** Compact format file - strings are stored in a LUT to avoid duplication. */
    Compact,
    /** Optimized format file - namespaces/keys are pre-hashed (CRC32), we know the number of elements up-front, and the
       number of references for each string in the LUT (to allow stealing). */
    Optimized_CRC32,
    /** Optimized format file - namespaces/keys are pre-hashed (CityHash64, UTF-16), we know the number of elements
       up-front, and the number of references for each string in the LUT (to allow stealing). */
    Optimized_CityHash64_UTF16,
  };

  constexpr FGuid LOC_RES_MAGIC{0x7574140Eu, 0xFC034A67u, 0x9D90154Au, 0x1B7F37C3u};

  class LocResParser {
  public:
    LocResParser(LocManager *manager, const UnifiedVfsEntry *vfsEntry);

    void parse();
    std::unordered_map<FTextKey, std::unordered_map<FTextKey, FEntry>> &getMap() { return entries; }

  private:
    std::unique_ptr<HeapStorage> m_storage;
    const UnifiedVfsEntry *m_vfsEntry;
    LocManager *m_manager = nullptr;
    std::span<FTextLocalizationResourceString> localizedStringArray;
    std::unordered_map<FTextKey, std::unordered_map<FTextKey, FEntry>> entries;
  };
} // namespace SAP
