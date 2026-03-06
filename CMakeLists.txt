cmake_minimum_required(VERSION 3.24)
project(SatisfactoryAssetParser LANGUAGES CXX)


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
        src/util/heap_pool.cpp
        src/util/heap_pool.hpp
        src/util/ThroughputStats.hpp
        src/util/PropertyTracker.hpp
        src/util/PropertyTracker.cpp)

target_sources(SAA_Util
        PUBLIC
        src/util/shared_cache.hpp
        src/util/byte_cursor.hpp
        src/util/thread_arena.hpp
        src/util/heap_storage.hpp
        PRIVATE
        src/util/byte_cursor.cpp
        src/util/thread_arena.cpp
        src/util/heap_storage.cpp
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
        $<LINK_LIBRARY:WHOLE_ARCHIVE,SAA_Assets> #DONT REMOVE !!!
        PRIVATE
)

# -------------------------------------------------------------------------
# 6. Executables
# -------------------------------------------------------------------------
add_executable(AssetRipperTest src/main.cpp)
target_link_libraries(AssetRipperTest PRIVATE SAA_Lib)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(MSVC)
        add_compile_options(/O2 /arch:AVX2 /fp:fast /GL)
        add_link_options(/LTCG)
    else()
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
                -static              # Linkt alles statisch
                -static-libgcc       # Speziell für die GCC Runtime
                -static-libstdc++    # Speziell für die C++ Standard Library
        )
    endif()

else()
    message(STATUS "SAA: Debug build detected - skipping heavy optimizations.")
    add_compile_options(-g -O0)
endif()


set_target_properties(AssetRipperTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        OUTPUT_NAME "AssetRipperTest"
)