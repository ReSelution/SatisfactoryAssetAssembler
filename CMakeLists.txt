cmake_minimum_required(VERSION 3.24)
project(SatisfactoryAssetParser LANGUAGES CXX)

# -------------------------------------------------------------------------
# 1. Globale Einstellungen & Performance
# -------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ordner-Struktur für Binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
    # Statisches Linken für portable Windows-Builds
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# -------------------------------------------------------------------------
# 2. Third Party (External)
# -------------------------------------------------------------------------
add_subdirectory(external/spdlog)
add_subdirectory(external/cityhash)
add_subdirectory(external/fastgltf)

# -------------------------------------------------------------------------
# 3. Core Module (Subdirectories)
# -------------------------------------------------------------------------
# Diese sollten ihre eigenen add_library() Aufrufe haben
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
add_library(SAP_Util STATIC)

target_sources(SAP_Util
        PUBLIC
        src/util/logger.hpp
        src/util/shared_cache.hpp
        src/util/byte_cursor.hpp
        src/util/thread_arena.hpp
        src/util/string_storage.hpp
        PRIVATE
        src/util/logger.cpp
        src/util/byte_cursor.cpp
        src/util/thread_arena.cpp
        src/util/string_storage.cpp
)

target_include_directories(SAP_Util PUBLIC src/util)
target_link_libraries(SAP_Util
        PUBLIC
        SAP_Structs
        SAP_Parser
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

set_target_properties(AssetRipperTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)