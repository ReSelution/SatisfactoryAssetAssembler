add_library(SAP_Crawler STATIC)

target_sources(SAP_Crawler
        PUBLIC
        crawler.hpp

)

target_sources(SAP_Crawler
        PRIVATE
        crawler.cpp
)

target_link_libraries(SAP_Crawler
        PUBLIC
        SAP_Assets
        SAP_Structs
        SAP_Reader
        SAP_Exporter
        SAP_VFS
)

target_include_directories(SAP_Crawler
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)