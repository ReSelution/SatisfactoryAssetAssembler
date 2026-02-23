add_library(SAP_Exporter STATIC
        puplic/arena/staging_arena.cpp
        puplic/arena/staging_arena.hpp
        puplic/arena/warehouse_arena.cpp
        puplic/arena/warehouse_arena.hpp
)

target_sources(SAP_Exporter
        PUBLIC
        exporter.hpp
        flags/TextureExportFlags.hpp
        texture/texture_exporter.hpp
        texture/texture_decoder.hpp
        meshes/mesh_exporter.hpp
        flags/MeshExportFlags.hpp
        material/material_exporter.hpp
        structs/gltf_material.hpp
        structs/exported_texture_data.hpp
)

target_sources(SAP_Exporter
        PRIVATE
        exporter.cpp
        texture/texture_exporter.cpp
        texture/texture_decoder.cpp
        meshes/mesh_exporter.cpp
        material/material_exporter.cpp
)

target_link_libraries(SAP_Exporter
        PUBLIC
        SAP_Assets
        SAP_Structs
        SAP_Reader
        fastgltf::fastgltf
)

target_include_directories(SAP_Exporter
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/external
)