cmake_minimum_required(VERSION 3.24)
project(SatisfactoryAssetParser LANGUAGES CXX)
include(CheckCXXCompilerFlag)

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
# -------------------------------------------------------------------------
# 1. Globale Einstellungen & Performance
# -------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


# -------------------------------------------------------------------------
# 2. Third Party (External)
# -------------------------------------------------------------------------
add_subdirectory(external/cityhash)
add_subdirectory(external/fastgltf)
add_subdirectory(external/StructuredChaos)

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
        src/util/stats.hpp)

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
add_library(SAA_Lib STATIC)

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
        SAA_Util
        SAA_Structs
        SAA_Parser
        SAA_Reader
        StructuredChaos
        SAA_Crawler
        # Whole Archive for Asset-Factory Registration
        $<LINK_LIBRARY:WHOLE_ARCHIVE,SAA_Assets>
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
    )
    set_target_properties(AssetRipperTest PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            OUTPUT_NAME "AssetRipperTest"
    )
else ()
    add_executable(AssetRipperTestWin src/main.cpp)
    target_link_libraries(AssetRipperTestWin PRIVATE
            SAA_Lib
            StructuredChaos
    )
    set_target_properties(AssetRipperTestWin PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            OUTPUT_NAME "AssetRipperTest"
            CROSSCOMPILING_EMULATOR "${WINE_EXECUTABLE}"
    )
    set_target_properties(AssetRipperTestWin PROPERTIES LINK_FLAGS "-Wl,--subsystem,console")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if (MSVC)
        add_compile_options(/O2 /arch:AVX2 /fp:fast /GL)
        add_link_options(/LTCG)
    else ()
        add_compile_options(
                -O3
                -march=native
                -flto
                -fno-exceptions
                -fno-rtti
                -ffast-math
        )
        add_link_options(
                -flto
        )
        if (SAA_TARGETING_WINDOWS AND SAA_HAS_LLD)
            add_link_options("-Wl,/opt:lldlto=2") # Speed up LTO linking on LLD if supported
        endif ()
    endif ()

else ()
    message(STATUS "SAA: Debug build detected - skipping heavy optimizations.")
    add_compile_options(-g -O0)
endif ()


