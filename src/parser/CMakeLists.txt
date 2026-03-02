add_library(SAA_Parser)

target_sources(SAA_Parser
    PUBLIC
        AssetFileParser.hpp
        AssetParser.hpp
        uasset_parser.hpp
)

target_sources(SAA_Parser
        PRIVATE
        AssetParser.cpp
        uasset_parser.cpp
        AssetFileParser.cpp
)

target_include_directories(SAA_Parser
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SAA_Parser
        PUBLIC
        SAA_Localization
        PRIVATE
        SAA_Structs
        SAA_Util
        SAA_Assets
        SAA_Lib
        SAA_VFS
)