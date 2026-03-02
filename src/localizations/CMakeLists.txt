 # ==============================================================================
 # SAA_Localization - Handling LocMeta and LocRes files
 # ==============================================================================
 add_library(SAA_Localization STATIC)

target_sources(SAA_Localization
    PUBLIC
    loc_manager.hpp
    loc_meta_parser.hpp
    loc_res_parser.hpp
    PRIVATE
    loc_manager.cpp
    loc_meta_parser.cpp
    loc_res_parser.cpp
)

target_include_directories(SAA_Localization
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

 target_link_libraries(SAA_Localization
        PUBLIC
        SAA_Structs
        SAA_Util
        SAA_VFS
 )
