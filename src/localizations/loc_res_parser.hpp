//
// Created by oleub on 06.02.26.
//

#pragma once

#include <cstdint>
#include <span>

#include <ankerl/unordered_dense.h>

#include "core/f_guid.hpp"

#include "localization/f_entry.hpp"
#include "localization/f_text_key.hpp"
#include "localization/f_text_localization_resource_string.hpp"
#include "sc/localizations/magpie_translator.hpp"

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
    LocResParser(const UnifiedVfsEntry *vfsEntry, bool isNative = false);

    void parse();

  private:
    bool isNative;
    const UnifiedVfsEntry *m_vfsEntry;
    std::span<FTextLocalizationResourceString> localizedStringArray;
    sc::Magpie *magpie;
  };
} 
