add_library(SAP_Parser)

target_sources(SAP_Parser
    PUBLIC
        AssetFileParser.hpp
        AssetParser.hpp
        uasset_parser.hpp
)

target_sources(SAP_Parser
        PRIVATE
        AssetParser.cpp
        uasset_parser.cpp
        AssetFileParser.cpp
)

target_include_directories(SAP_Parser
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SAP_Parser
        PRIVATE
        SAP_Structs
        SAP_Util
        SAP_Assets
        SAP_Lib
        SAP_VFS
        SAP_Localization
)