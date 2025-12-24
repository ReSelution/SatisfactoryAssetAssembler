
add_library(SAP_Assets STATIC)

target_sources(SAP_Assets
        PUBLIC
        ue/object_factory.hpp
        ue/asset_object.hpp
        ue/textures/texture_2d.hpp
        ue/textures/texture.hpp

        PRIVATE
        ue/object_factory.cpp
        ue/asset_object.cpp
        ue/textures/texture.cpp
        ue/textures/texture_2d.cpp
)

target_link_libraries(SAP_Assets
        PRIVATE
        SAP_Structs
        SAP_Parser
        SAP_Util
        SAP_Lib
)

target_include_directories(SAP_Assets
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE

)
