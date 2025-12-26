add_library(SAP_Exporter STATIC
        flags/TextureExportFlags.hpp
        texture_exporter.cpp
        texture_exporter.hpp)

target_sources(SAP_Exporter
        PUBLIC
        exporter.hpp
)

target_sources(SAP_Exporter
        PRIVATE
        exporter.cpp
)

target_link_libraries(SAP_Exporter
        PUBLIC
        SAP_Assets
        SAP_Structs
)

target_include_directories(SAP_Exporter
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)