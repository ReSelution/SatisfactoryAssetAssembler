//
// Created by oleub on 06.02.26.
//

#pragma once

#include <memory>
#include <random>


#include "core/f_guid.hpp"
#include "memory/chaos_arena_pool.hpp"
#include "hash/hash.hpp"
#include <ankerl/unordered_dense.h>


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
        using denseMap = ankerl::unordered_dense::map<std::string_view, std::string_view, SC::IdentityHash>;
        denseMap cultures;
        std::unique_ptr<SC::ChaosBumpArena> m_storage{};

        std::string nativeLocResPATH;
        std::string nativeCulture;
    };
} 
