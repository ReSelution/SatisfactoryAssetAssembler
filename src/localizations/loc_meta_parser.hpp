//
// Created by oleub on 06.02.26.
//

#pragma once
#include <deque>
#include <memory>
#include <random>
#include <unordered_map>

#include "string_storage.hpp"

#include "core/f_guid.hpp"

namespace SAP {
  class UnifiedVfsEntry;
  class LocManager;
  enum class ELocMetaVersion : uint8_t {
    /** Initial format. */
    Initial = 0,
    /** Added complete list of cultures compiled for the localization target. */
    AddedCompiledCultures,
    /** Added bIsUGC flag */
    AddedIsUGC,
  };

  constexpr FGuid LOC_META_MAGIC{0xA14CEE4Fu, 0x83554868u, 0xBD464C6Cu, 0x7C50DA70u};

  class LocMetaParser {
  public:
    LocMetaParser(LocManager *manager, const UnifiedVfsEntry* vfsEntry);
    void parse();
    std::string_view getNativeRes() const {return nativeLocResPATH;};
    std::pair<bool, std::string_view> getRes(std::string_view culture) const;


  private:
    std::vector<uint8_t> data;

    const UnifiedVfsEntry* m_vfsEntry;
    LocManager *m_manager;
    std::unordered_map<std::string, std::string> cultures;
    HeapStorage<> m_stringStorage{};

    std::string nativeLocResPATH;
    std::string nativeCulture;
  };
} // SAP
