//
// Created by oleub on 19.02.26.
//

#include "PropertyTracker.hpp"

#include "ue/properties/f_property_tag.hpp"

#include "sc/logger/logger.hpp"
using PropertyTrackerLog = sc::Logger<"SAA", "Tracker">;

namespace SAA {
#ifndef NDEBUG

  std::map<std::string, std::map<uint64_t, PropertyInfo>> PropertyTracker::m_allSeen{};

  void PropertyTracker::registerMap(std::string_view className, denseMap *props) {
    if (className == "Struct") {
      return;
    }
    std::string cls(className);
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_classInstanceCount[cls];
    auto &classData = m_allSeen[cls];

    for (auto &[key, tag]: *props) {
      auto &info = classData[key];
      if (info.count == 0) {
        info.tag = tag;
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

          PropertyTrackerLog::warn("{} : {}/{} -> {}", prop.tag.Name.text(), prop.count, count,
                                   prop.tag.PropertyTypeName.text());
        }
      }
    }
  }
#endif
} // namespace SAA
