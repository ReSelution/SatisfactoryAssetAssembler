cmake_minimum_required(VERSION 3.24)
project(SatisfactoryAssetParser LANGUAGES CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "ccache gefunden und aktiviert.")
endif()
include(CheckCXXCompilerFlag)

# -------------------------------------------------------------------------
# 1. Globale Einstellungen & Performance
# -------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(ENABLE_ASAN "Enable AddressSanitizer (ASan)" OFF)


if (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SAA_TARGETING_WINDOWS TRUE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_link_options("-static")
        add_link_options("-static-libgcc")
        add_link_options("-static-libstdc++")
    endif ()
else ()
    set(SAA_TARGETING_WINDOWS FALSE)
endif ()

if (NOT MSVC)
    include(CheckCXXCompilerFlag)

    set(_SAA_LINKER "Default (Slow)")

    if (NOT SAA_TARGETING_WINDOWS)
        check_cxx_compiler_flag("-fuse-ld=mold" SAA_HAS_MOLD)
        check_cxx_compiler_flag("-fuse-ld=lld" SAA_HAS_LLD)
    endif ()

    if (SAA_HAS_MOLD AND NOT SAA_TARGETING_WINDOWS)
        set(_SAA_LINKER "mold")
        add_link_options("-fuse-ld=mold")
    elseif (SAA_HAS_LLD AND NOT SAA_TARGETING_WINDOWS)
        set(_SAA_LINKER "lld")
        add_link_options("-fuse-ld=lld")
    endif ()

    message(STATUS "[SAA] Using Linker: ${_SAA_LINKER}")
endif ()

set(SAA_SANITIZER_FLAGS "")
if (ENABLE_ASAN)
    if (MSVC)
        set(SAA_SANITIZER_FLAGS "/fsanitize=address")
        add_link_options("/incremental:no")
        message(STATUS "[SAA] ASan enabled for MSVC")
    else ()
        set(SAA_SANITIZER_FLAGS "-fsanitize=address" "-fno-omit-frame-pointer")
        message(STATUS "[SAA] ASan enabled for GCC/Clang")
    endif ()

    # Apply to all targets automatically
    add_compile_options(${SAA_SANITIZER_FLAGS})
    add_link_options(${SAA_SANITIZER_FLAGS})
endif ()

add_library(SAA_Lib STATIC)

if (NOT MSVC)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
    if(supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO/LTO is not supported: ${error}")
    endif()

    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(SAA_Lib PUBLIC  -O3 -march=native -fno-rtti)
    else()
        target_compile_options(SAA_Lib PUBLIC -g -O0)
        message(STATUS "[SAA] Debug mode: Optimizations disabled")
    endif()
endif()

# -------------------------------------------------------------------------
# 2. Third Party (External)
# -------------------------------------------------------------------------
if(NOT TARGET StructuredChaos)
add_subdirectory(external/StructuredChaos)
endif ()
add_subdirectory(external/cityhash)
include(cmake/dependencies.cmake)





# -------------------------------------------------------------------------
# 3. Core Module (Subdirectories)
# -------------------------------------------------------------------------
add_subdirectory(src/structs)
add_subdirectory(src/parser)
add_subdirectory(src/assets)
add_subdirectory(src/reader)
add_subdirectory(src/exporter)
add_subdirectory(src/crawler)
add_subdirectory(src/vfs)
add_subdirectory(src/localizations)

# -------------------------------------------------------------------------
# 4. SAA_Util: Basis-Infrastruktur
# -------------------------------------------------------------------------
add_library(SAA_Util STATIC

        src/util/PropertyTracker.hpp
        src/util/PropertyTracker.cpp
        src/util/stats.hpp
)

target_sources(SAA_Util
        PUBLIC
        src/util/shared_cache.hpp
        src/util/byte_cursor.hpp
        PRIVATE
        src/util/byte_cursor.cpp
)

target_include_directories(SAA_Util PUBLIC src/util)
target_link_libraries(SAA_Util
        PUBLIC
        SAA_Structs
        SAA_Parser
        SAA_Assets
        spdlog::spdlog
)
# -------------------------------------------------------------------------
# 5. SAA_Lib: Der "High-Level" Wrapper & Provider
# -------------------------------------------------------------------------


target_sources(SAA_Lib
        PUBLIC
        src/provider/games/satisfactory/SatisfactoryDataProvider.hpp
        src/decompress/Decompressor.hpp
        src/decompress/oodle_loader.hpp

        PRIVATE
        src/provider/games/satisfactory/SatisfactoryDataProvider.cpp
        src/decompress/Decompressor.cpp
        src/decompress/oodle_loader.cpp
)

target_include_directories(SAA_Lib
        PUBLIC
        src/provider
        src/decompress
)

target_link_libraries(SAA_Lib
        PUBLIC
        SAA_Parser
        SAA_Reader
        SAA_Crawler

        # Whole Archive for Asset-Factory Registration
        $<LINK_LIBRARY:WHOLE_ARCHIVE,SAA_Assets>
        StructuredChaos
        SAA_Util
        SAA_Structs
        PRIVATE
)

# -------------------------------------------------------------------------
# 6. Executables
# -------------------------------------------------------------------------

if (NOT SAA_TARGETING_WINDOWS)
    add_executable(AssetRipperTest src/main.cpp)
    target_link_libraries(AssetRipperTest PRIVATE
            SAA_Lib
            StructuredChaos
            mimalloc-static
    )
    set_target_properties(AssetRipperTest PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
            OUTPUT_NAME "AssetRipperTest-${CMAKE_BUILD_TYPE}"
    )
else ()
    add_executable(AssetRipperTestWin src/main.cpp)
    target_link_libraries(AssetRipperTestWin PRIVATE
            SAA_Lib
            StructuredChaos
            mimalloc-static
    )
    set_target_properties(AssetRipperTestWin PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
            OUTPUT_NAME "AssetRipperTest-${CMAKE_BUILD_TYPE}"
    )
    set_target_properties(AssetRipperTestWin PROPERTIES LINK_FLAGS "-Wl,--subsystem,console")

endif ()


