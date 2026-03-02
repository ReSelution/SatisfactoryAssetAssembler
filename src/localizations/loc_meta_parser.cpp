//
// Created by oleub on 06.02.26.
//

#include "loc_meta_parser.hpp"

#include <utility>

#include "byte_cursor.hpp"
#include "loc_manager.hpp"
#include "logger.hpp"
#include "vfs_entry.hpp"

namespace SAA {
  std::string cleanPath(std::string path) {
    constexpr std::string_view posixSuffix = "-POSIX";
    size_t pos = path.find(posixSuffix);

    if (pos != std::string::npos) {
      path.erase(pos, posixSuffix.length());
    }
    return path;
  }

  LocMetaParser::LocMetaParser(LocManager *manager, const UnifiedVfsEntry *vfsEntry) :
      m_manager(manager), m_vfsEntry(vfsEntry) {}

  void LocMetaParser::parse() {
    m_storage = std::make_unique<HeapStorage>(512);
    auto data = m_vfsEntry->read(m_storage.get());
    ByteCursor cursor(data);
    auto magic = cursor.read<FGuid>();
    if (magic != LOC_META_MAGIC) {
      Logger::logError("[LOCMETA]", "{} found: A: {} B: {} C: {} D: {}", m_vfsEntry->path, magic.A, magic.B, magic.C,
                       magic.D);
      return;
    }
    auto version = cursor.read<ELocMetaVersion>();
    nativeCulture = cursor.readFString(m_storage.get());
    m_manager->registerCulture(nativeCulture);
    auto nativeLocRes = cursor.readFString(m_storage.get());

    auto pos = m_vfsEntry->path.find_last_of('/');
    auto prefix = m_vfsEntry->path.substr(0, pos + 1);

    nativeLocResPATH.reserve(prefix.size() + nativeLocRes.size());
    nativeLocResPATH.append(prefix).append(nativeLocRes);
    nativeLocResPATH = cleanPath(std::move(nativeLocResPATH));

    if (version >= ELocMetaVersion::AddedCompiledCultures) {
      std::string_view suffix = nativeLocRes.substr(nativeCulture.size());
      auto count = cursor.read<uint32_t>();
      cultures.reserve(count);
      for (auto i = 0; i < count; i++) {
        auto currentCulture = cursor.readFString(m_storage.get());
        m_manager->registerCulture(currentCulture);
        std::string path;
        path.reserve(prefix.size() + currentCulture.size() + suffix.size());
        path.append(prefix).append(currentCulture).append(suffix);
        cultures.emplace(currentCulture, cleanPath(path));
      }
    }
    return;
  }

  // bool if it is the nativ Culture
  std::pair<bool, std::string_view> LocMetaParser::getRes(std::string_view culture) const {
    auto cult = cultures.find(std::string(culture));
    if (cult != cultures.end()) {
      return {culture == nativeCulture, cult->second};
    }

    return {false, ""};
  }
} // namespace SAP
