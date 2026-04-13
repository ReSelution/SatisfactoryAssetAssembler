add_library(SAA_Exporter STATIC
        puplic/arena/registery.cpp
        puplic/arena/registery.hpp
        puplic/structs/gpu_export.hpp
        puplic/structs/meta_export.hpp

)

target_sources(SAA_Exporter
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
        fg/fg_exporter.hpp
)

target_sources(SAA_Exporter
        PRIVATE
        exporter.cpp
        texture/texture_exporter.cpp
        texture/texture_decoder.cpp
        meshes/mesh_exporter.cpp
        material/material_exporter.cpp
        fg/fg_exporter.cpp

)

target_link_libraries(SAA_Exporter
        PUBLIC
        SAA_Assets
        SAA_Structs
        SAA_Reader
        fastgltf::fastgltf
        EnTT
        StructuredChaos
)

target_include_directories(SAA_Exporter
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/external
)