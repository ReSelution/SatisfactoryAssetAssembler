//
// Created by oleub on 19.02.26.
//

#include "PropertyTracker.hpp"

#include "ue/properties/f_property_tag.hpp"

#include "sc/logger/logger.hpp"
using PropertyTrackerLog = sc::Logger<"SAA", "Tracker">;

namespace SAA {
#ifdef SAA_TRACK_PROPERTIES

  std::map<std::string, std::map<uint64_t, PropertyInfo>> PropertyTracker::m_allSeen{};

  void PropertyTracker::registerMap(std::string_view className, const PropertyMap *props) {
    if (className == "Struct" || props == nullptr) {
      return;
    }
    std::string cls(className);
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_classInstanceCount[cls];
    auto &classData = m_allSeen[cls];

    const auto keys   = props->keys();
    const auto values = props->values();
    for (size_t i = 0; i < keys.size(); ++i) {
      auto &info = classData[keys[i]];
      if (info.count == 0) {
        info.tag = values[i];
      }
      ++info.count;
    }
  }

  void PropertyTracker::printMissing() {
    bool anyMissingAtAll = false;

    for (auto &[className, seen]: m_allSeen) {
      const auto &handled     = m_handled[className];
      bool classHeaderPrinted = false;
      auto &count             = m_classInstanceCount[className];
      for (auto &[hash, prop]: seen) {
        if (handled.find(hash) == handled.end()) {
          if (!anyMissingAtAll) {
            PropertyTrackerLog::info("=== missing Property-Mappings ===");
            anyMissingAtAll = true;
          }

          if (!classHeaderPrinted) {
            PropertyTrackerLog::err("Class: {}", className);
            classHeaderPrinted = true;
          }

          PropertyTrackerLog::warn("{} : {}/{} -> {}", prop.tag.Name, prop.count, count, prop.tag.TypeName);
        }
      }
    }
  }
#endif
} // namespace SAA
