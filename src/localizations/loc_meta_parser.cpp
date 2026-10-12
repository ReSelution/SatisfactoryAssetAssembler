//
// Created by oleub on 06.02.26.
//

#include "loc_meta_parser.hpp"

#include <cstring>
#include <utility>

#include "byte_cursor.hpp"
#include "game_file.hpp"
#include "loc_manager.hpp"
#include "sc/logger/logger.hpp"
#include "sc/memory/monotonic_resource.hpp"
#include "vfs_entry.hpp"

using LocMetaLog = sc::Logger<"SAA", "LOCMETA">;

namespace SAA {
  std::string_view cleanPath(std::string_view path) {
    constexpr std::string_view posixSuffix = "-POSIX";
    size_t pos = path.find(posixSuffix);
    if (pos != std::string_view ::npos) {
      path = std::string_view {path.data(), pos};
    }
    return path;
  }

  LocMetaParser::LocMetaParser(LocManager *manager, const UnifiedVfsEntry *vfsEntry) :
      m_vfsEntry(vfsEntry), m_manager(manager) {}

  void LocMetaParser::parse() {
    m_storage = std::make_unique<sc::MonotonicResource>(1024);
    auto data = m_vfsEntry->read(m_storage.get());

    ByteCursor cursor(data->subspan(0, data->size()));
    auto magic = cursor.read<FGuid>();
    if (magic != LOC_META_MAGIC) {
      LocMetaLog::err("{} found: A: {} B: {} C: {} D: {}", m_vfsEntry->pathStr, magic.A, magic.B, magic.C,
                       magic.D);
      return;
    }
    auto version = cursor.read<ELocMetaVersion>();
    nativeCulture = cursor.readFString(m_storage.get());
    m_manager->registerCulture(nativeCulture);
    auto nativeLocRes = cursor.readFString(m_storage.get());

    auto pos = m_vfsEntry->pathStr.find_last_of('/');
    auto prefix = m_vfsEntry->pathStr.substr(0, pos + 1);

    nativeLocResPATH.reserve(prefix.size() + nativeLocRes.size());
    nativeLocResPATH.append(prefix).append(nativeLocRes);
    nativeLocResPATH = cleanPath(std::move(nativeLocResPATH));

    if (version >= ELocMetaVersion::AddedCompiledCultures) {
      std::string_view suffix = nativeLocRes.substr(nativeCulture.size());
      auto count = cursor.read<uint32_t>();
      cultures.reserve(count);
      for (uint32_t i = 0; i < count; i++) {
        auto currentCulture = cursor.readFString(m_storage.get());
        m_manager->registerCulture(currentCulture);

        size_t totalSize = prefix.size() + currentCulture.size() + suffix.size();
        // Built in the arena right away. cleanPath() only cuts the end off, the view stays inside arenaMem.
        auto arenaMem = m_storage->allocateSpan<char>(totalSize);
        std::memcpy(arenaMem.data(), prefix.data(), prefix.size());
        std::memcpy(arenaMem.data() + prefix.size(), currentCulture.data(), currentCulture.size());
        std::memcpy(arenaMem.data() + prefix.size() + currentCulture.size(), suffix.data(), suffix.size());
        std::string_view persistentPath = cleanPath({arenaMem.data(), totalSize});

        cultures.emplace(currentCulture, persistentPath);
      }
    }
  }

  // bool if it is the nativ Culture
  std::pair<bool, std::string_view> LocMetaParser::getRes(std::string_view culture) const {
    auto cult = cultures.find(std::string(culture));
    if (cult != cultures.end()) {
      return {culture == nativeCulture, cult->second};
    }

    return {false, ""};
  }
} 
