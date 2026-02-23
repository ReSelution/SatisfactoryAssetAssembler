cmake_minimum_required(VERSION 3.24)
project(SatisfactoryAssetParser LANGUAGES CXX)
if(NOT CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
endif()
include(GNUInstallDirs)
# -------------------------------------------------------------------------
# 1. Globale Einstellungen & Performance
# -------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


# -------------------------------------------------------------------------
# 2. Third Party (External)
# -------------------------------------------------------------------------
add_subdirectory(external/spdlog)
add_subdirectory(external/cityhash)
add_subdirectory(external/fastgltf)

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
# 4. SAP_Util: Basis-Infrastruktur
# -------------------------------------------------------------------------
add_library(SAP_Util STATIC
        src/util/heap_pool.cpp
        src/util/heap_pool.hpp
        src/util/ThroughputStats.hpp
        src/util/PropertyTracker.hpp
        src/util/PropertyTracker.cpp)

target_sources(SAP_Util
        PUBLIC
        src/util/logger.hpp
        src/util/shared_cache.hpp
        src/util/byte_cursor.hpp
        src/util/thread_arena.hpp
        src/util/heap_storage.hpp
        PRIVATE
        src/util/logger.cpp
        src/util/byte_cursor.cpp
        src/util/thread_arena.cpp
        src/util/heap_storage.cpp
)

target_include_directories(SAP_Util PUBLIC src/util)
target_link_libraries(SAP_Util
        PUBLIC
        SAP_Structs
        SAP_Parser
        SAP_Assets
        spdlog::spdlog
)
# -------------------------------------------------------------------------
# 5. SAP_Lib: Der "High-Level" Wrapper & Provider
# -------------------------------------------------------------------------
add_library(SAP_Lib STATIC)

target_sources(SAP_Lib
        PUBLIC
        src/provider/games/satisfactory/SatisfactoryDataProvider.hpp
        src/decompress/Decompressor.hpp
        src/decompress/oodle_loader.hpp

        PRIVATE
        src/provider/games/satisfactory/SatisfactoryDataProvider.cpp
        src/decompress/Decompressor.cpp
        src/decompress/oodle_loader.cpp
)

target_include_directories(SAP_Lib
        PUBLIC
        src/provider
        src/decompress
)

target_link_libraries(SAP_Lib
        PUBLIC
        SAP_Util
        SAP_Structs
        SAP_Parser
        SAP_Reader
        SAP_Crawler
        # Whole Archive for Asset-Factory Registration
        $<LINK_LIBRARY:WHOLE_ARCHIVE,SAP_Assets> #DONT REMOVE !!!
        PRIVATE
        spdlog::spdlog
)

# -------------------------------------------------------------------------
# 6. Executables
# -------------------------------------------------------------------------
add_executable(AssetRipperTest src/main.cpp)
target_link_libraries(AssetRipperTest PRIVATE SAP_Lib)

if(WIN32)
    target_link_libraries(AssetRipperTest PRIVATE
            "-static-libgcc"
            "-static-libstdc++"
            "-static"
            "pthread"
    )
    set_target_properties(AssetRipperTest PROPERTIES
            LINK_FLAGS "-static -static-libgcc -static-libstdc++"
    )
endif()


set_target_properties(AssetRipperTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)