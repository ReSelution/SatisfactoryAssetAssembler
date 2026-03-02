add_library(SAA_Crawler STATIC)

target_sources(SAA_Crawler
        PUBLIC
        crawler.hpp

)

target_sources(SAA_Crawler
        PRIVATE
        crawler.cpp
)

target_link_libraries(SAA_Crawler
        PUBLIC
        SAA_Assets
        SAA_Structs
        SAA_Reader
        SAA_Exporter
        SAA_VFS
)

target_include_directories(SAA_Crawler
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)