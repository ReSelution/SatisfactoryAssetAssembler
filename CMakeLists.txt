cmake_minimum_required(VERSION 3.24)

project(SatisfactoryAssetParser LANGUAGES CXX)

# -------------------------------------------------------------------------
# General settings
# -------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Third party libs
add_subdirectory(external/spdlog)
add_subdirectory(external/cityhash)



# -------------------------------------------------------------------------
# Function: create_resources
# -------------------------------------------------------------------------
function(create_resources dir output)
    file(WRITE ${output} "")

    set(binaries
            "${dir}/liboo2corelinux64.so.9"
            "${dir}/oo2core_9_win64.dll"
    )

    foreach (bin IN LISTS binaries)
        if (EXISTS ${bin})
            get_filename_component(filename ${bin} NAME)

            string(REGEX REPLACE "\\.| |-" "_" c_symbol ${filename})

            file(READ ${bin} filedata HEX)

            string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})

            file(APPEND ${output}
                    "extern \"C\" const unsigned char ${c_symbol}[] = { ${filedata} };\n"
            )

            file(APPEND ${output}
                    "extern \"C\" const unsigned ${c_symbol}_size = sizeof(${c_symbol});\n\n"
            )

            message(STATUS "Embedded ${filename} as ${c_symbol}")
        else ()
            message(WARNING "Missing Oodle binary: ${bin}")
        endif ()
    endforeach ()
endfunction()

# Generated resource C++ file
set(OODLE_GENERATED_FILE ${CMAKE_CURRENT_BINARY_DIR}/OodleBinary_Generated.cpp)
create_resources(${CMAKE_SOURCE_DIR} ${OODLE_GENERATED_FILE})


# -------------------------------------------------------------------------
# Library: SAP_Util (Logger, Oodle)
# -------------------------------------------------------------------------
add_library(SAP_Util STATIC)
target_sources(SAP_Util
        PUBLIC
        src/util/logger.hpp

        PRIVATE
        src/util/logger.cpp

)
target_include_directories(SAP_Util PUBLIC src/util)

# Libs
add_subdirectory(src/structs)
add_subdirectory(src/parser)
add_subdirectory(src/assets)
add_subdirectory(src/reader)
add_subdirectory(src/exporter)


add_library(SAP_Lib STATIC)
target_sources(SAP_Lib
        PRIVATE
        ${OODLE_GENERATED_FILE}
        src/provider/games/satisfactory/SatisfactoryDataProvider.cpp

        src/vfs/vfs.cpp
        src/decompress/Decompressor.cpp

)

target_sources(SAP_Lib
        PUBLIC
        src/provider/games/satisfactory/SatisfactoryDataProvider.hpp

        src/decompress/Decompressor.hpp
        src/decompress/OodleEmbedded.hpp

        src/vfs/vfs.hpp
)


target_link_libraries(SAP_Lib
        PUBLIC
        SAP_Util
        SAP_Structs
        SAP_Parser
        SAP_Assets
        SAP_Reader
        $<LINK_LIBRARY:WHOLE_ARCHIVE,SAP_Assets> #Needed for Factory Class Registration
        PRIVATE
        spdlog::spdlog
)

target_include_directories(SAP_Lib
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src/provider
        ${CMAKE_SOURCE_DIR}/src/vfs
        ${CMAKE_SOURCE_DIR}/src/decompress
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}   # for generated file
)

# -------------------------------------------------------------------------
# Executable: AssetRipperTest
# -------------------------------------------------------------------------
add_executable(AssetRipperTest src/main.cpp)

target_link_libraries(AssetRipperTest PRIVATE SAP_Lib)

set_target_properties(AssetRipperTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
