#pragma once
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>


namespace SAA {
#ifndef NDEBUG
    struct PropertyInfo;
    struct FPropertyTag;

    class PropertyTracker {
    public:
        static void registerMap(std::string_view className, std::pmr::unordered_map<uint32_t, FPropertyTag> &props);

        static void markHandled(std::string_view className, uint64_t propHash) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_handled[std::string(className)].insert(propHash);
        }

        static void printMissing();

    private:
        static inline std::mutex m_mutex;
        static std::map<std::string, std::map<uint64_t, PropertyInfo> > m_allSeen;
        static inline std::map<std::string, std::set<uint64_t> > m_handled;
        static inline std::map<std::string, size_t> m_classInstanceCount;
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
