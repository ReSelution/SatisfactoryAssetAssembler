#pragma once
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <string_view>

#include "logger.hpp"

namespace SAP {

#ifndef NDEBUG
    class PropertyTracker {
    public:
        // Wir nutzen ein Template, um flexibel bei der Map-Art zu bleiben
        template<typename MapType>
        static void registerMap(std::string_view className, MapType& props) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto& classData = m_allSeen[std::string(className)];

            for (auto &[key, tag] : props) {
                classData[key] = tag;
            }
        }

        static void markHandled(std::string_view className, uint64_t propHash) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_handled[std::string(className)].insert(propHash);
        }

      static void printMissing() {
          bool anyMissingAtAll = false;

          for (auto &[className, seen] : m_allSeen) {
            const auto& handled = m_handled[className];
            bool classHeaderPrinted = false;

            for (auto &[hash, prop] : seen) {
              if (handled.find(hash) == handled.end()) {
                if (!anyMissingAtAll) {
                  Logger::logInfo("[Tracker]", "=== Fehlende Property-Mappings ===");
                  anyMissingAtAll = true;
                }

                if (!classHeaderPrinted) {
                  Logger::logInfo("[Tracker]", "Klasse: {}", className);
                  classHeaderPrinted = true;
                }

                Logger::logWarn("[Tracker]", "{} -> {}", prop.Name.text(), prop.PropertyTypeName.text());
              }
            }
          }
        }

    private:
        static inline std::mutex m_mutex;
        static inline std::map<std::string, std::map<uint64_t, FPropertyTag>> m_allSeen;
        static inline std::map<std::string, std::set<uint64_t>> m_handled;
    };

    #define DBG_TRACK_MAP(cls, map) PropertyTracker::registerMap(cls, map)
    #define DBG_MARK_HANDLED(cls, hash) PropertyTracker::markHandled(cls, hash)
    #define DBG_PRINT_MISSING() PropertyTracker::printMissing()
#else
    #define DBG_TRACK_MAP(cls, map)
    #define DBG_MARK_HANDLED(cls, hash)
    #define DBG_PRINT_MISSING()
#endif
}