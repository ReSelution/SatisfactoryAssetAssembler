//
// Created by oleub on 06.02.26.
//

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <ankerl/unordered_dense.h>

#include "core/f_guid.hpp"
#include "sc/hash/hash.hpp"
#include "sc/memory/monotonic_resource.hpp"


namespace SAA {
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
        LocMetaParser(LocManager *manager, const UnifiedVfsEntry *vfsEntry);

        void parse();

        std::string_view getNativeRes() const { return nativeLocResPATH; };

        std::pair<bool, std::string_view> getRes(std::string_view culture) const;

    private:

        const UnifiedVfsEntry *m_vfsEntry;
        LocManager *m_manager;
        using denseMap = ankerl::unordered_dense::map<std::string_view, std::string_view, sc::TransparentHash>;
        denseMap cultures;
        // unique_ptr, weil sc::MonotonicResource nicht verschiebbar ist, LocMetaParser aber in einem std::vector liegt.
        std::unique_ptr<sc::MonotonicResource> m_storage{};

        std::string nativeLocResPATH;
        std::string nativeCulture;
    };
} 
