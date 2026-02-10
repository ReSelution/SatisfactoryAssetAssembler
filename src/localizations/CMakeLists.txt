 # ==============================================================================
 # SAP_Localization - Handling LocMeta and LocRes files
 # ==============================================================================
 add_library(SAP_Localization STATIC)

target_sources(SAP_Localization
    PUBLIC
    loc_manager.hpp
    loc_meta_parser.hpp
    loc_res_parser.hpp
    PRIVATE
    loc_manager.cpp
    loc_meta_parser.cpp
    loc_res_parser.cpp
)

target_include_directories(SAP_Localization
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

 target_link_libraries(SAP_Localization
        PUBLIC
        SAP_Structs
        SAP_Util
        SAP_VFS
 )
