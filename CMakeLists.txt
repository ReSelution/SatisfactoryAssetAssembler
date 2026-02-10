cmake_minimum_required(VERSION 3.24)

project(SatisfactoryAssetParser LANGUAGES CXX)

# -------------------------------------------------------------------------
# General settings
# -------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#set(CMAKE_CXX_FLAGS_DEBUG
#        "-fsanitize=address -fno-omit-frame-pointer -g -O1"
#)
#
#set(CMAKE_LINKER_FLAGS_DEBUG
#        "-fsanitize=address"
#)

if(WIN32)

    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# -------------------------------------------------------------------------
# Third party libs
# -------------------------------------------------------------------------
add_subdirectory(external/spdlog)
add_subdirectory(external/cityhash)
add_subdirectory(external/fastgltf)

# -------------------------------------------------------------------------
# Libs (Subdirectories)
# -------------------------------------------------------------------------
add_subdirectory(src/structs)
add_subdirectory(src/parser)
add_subdirectory(src/assets)
add_subdirectory(src/reader)
add_subdirectory(src/exporter)
add_subdirectory(src/crawler)

# -------------------------------------------------------------------------
# Library: SAP_Util (Logger, Oodle)
# -------------------------------------------------------------------------
add_library(SAP_Util STATIC
        src/util/thread_arena.cpp
        src/util/thread_arena.hpp
        src/util/string_storage.cpp
        src/util/string_storage.hpp
)
target_sources(SAP_Util
        PUBLIC
        src/util/logger.hpp
        src/util/shared_cache.hpp
        src/util/byte_cursor.hpp
        PRIVATE
        src/util/logger.cpp
        src/util/byte_cursor.cpp
)
target_include_directories(SAP_Util PUBLIC src/util)

target_link_libraries(SAP_Util
        PUBLIC
        SAP_Structs
        SAP_Parser
        spdlog::spdlog
)

add_library(SAP_Lib STATIC
        src/decompress/oodle_loader.cpp
        src/decompress/oodle_loader.hpp

)

target_sources(SAP_Lib
        PRIVATE
        src/provider/games/satisfactory/SatisfactoryDataProvider.cpp
        src/vfs/vfs_entry.cpp

        src/vfs/vfs.cpp
        src/decompress/Decompressor.cpp

        src/localizations/loc_manager.cpp
        src/localizations/loc_res_parser.cpp
        src/localizations/loc_meta_parser.cpp

)

target_sources(SAP_Lib
        PUBLIC
        src/provider/games/satisfactory/SatisfactoryDataProvider.hpp
        src/decompress/Decompressor.hpp
        src/vfs/vfs.hpp
        src/vfs/vfs_entry.hpp
        src/localizations/loc_manager.hpp
        src/localizations/loc_meta_parser.hpp
        src/localizations/loc_res_parser.hpp

)


target_link_libraries(SAP_Lib
        PUBLIC
        SAP_Util
        SAP_Structs
        SAP_Parser
        SAP_Assets
        SAP_Reader
        SAP_Crawler
        $<LINK_LIBRARY:WHOLE_ARCHIVE,SAP_Assets> #Needed for Factory Class Registration
        PRIVATE
        spdlog::spdlog
        CITY_HASH
)

target_include_directories(SAP_Lib
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src/provider
        ${CMAKE_SOURCE_DIR}/src/vfs
        ${CMAKE_SOURCE_DIR}/src/decompress
        PRIVATE
        external/cityhash
)

# -------------------------------------------------------------------------
# Executable: AssetRipperTest
# -------------------------------------------------------------------------
add_executable(AssetRipperTest src/main.cpp


)

target_link_libraries(AssetRipperTest PRIVATE SAP_Lib)

set_target_properties(AssetRipperTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
