# ----------------------------------------
# SAA_Assets Library Definition
# ----------------------------------------
add_library(SAA_Assets STATIC
        ue/controlRig/rig_hierarchy.cpp
        ue/controlRig/rig_hierarchy.hpp
        ue/rigVM/rig_vm.cpp
        ue/rigVM/rig_vm.hpp
        ue/properties/property_parser.hpp
        ue/properties/property_parser_1009.cpp
        ue/properties/property_parser_1017.cpp
        ue/properties/soft_object.cpp
        ue/properties/soft_object.hpp
        satisfactory/items/fg_item_base.cpp
        satisfactory/items/fg_item_base.hpp
        satisfactory/fg_factory.cpp
        satisfactory/fg_factory.hpp
        satisfactory/building/fg_building_base.cpp
        satisfactory/building/fg_building_base.hpp
        satisfactory/fg_descriptor_base.cpp
        satisfactory/fg_descriptor_base.hpp
        satisfactory/fg_equipment_descriptor.cpp
        satisfactory/fg_equipment_descriptor.hpp)

target_sources(SAA_Assets
        PUBLIC
        asset_object.hpp
        object_factory.hpp
        ue/object/field.hpp
        ue/object/struct.hpp
        ue/object/class.hpp
        ue/engine/blueprint_generated_class.hpp
        ue/properties/f_property_tag.hpp
        ue/properties/property_holder.hpp
        ue/static_mesh/static_mesh.hpp
        ue/textures/texture_2d.hpp
        ue/textures/texture.hpp
)

target_sources(SAA_Assets
        PRIVATE
        asset_object.cpp
        object_factory.cpp
        ue/object/field.cpp
        ue/object/struct.cpp
        ue/object/class.cpp
        ue/engine/blueprint_generated_class.cpp
        ue/properties/property_holder.cpp
        ue/static_mesh/static_mesh.cpp
        ue/static_mesh/static_mesh_socket.cpp
        ue/static_mesh/static_mesh_socket.hpp

        # Materials & Textures
        ue/material/unreal_material.cpp
        ue/material/unreal_material.hpp
        ue/material/material_interface.cpp
        ue/material/material_interface.hpp
        ue/material/material.cpp
        ue/material/material.hpp
        ue/material/material_instance.cpp
        ue/material/material_instance.hpp
        ue/material/material_instance_constant.cpp
        ue/material/material_instance_constant.hpp
        ue/material/f_material_texture_info.hpp
        ue/textures/texture.cpp
        ue/textures/texture_2d.cpp
        ue/textures/texture_2d_array.cpp
        ue/textures/texture_2d_array.hpp
        ue/textures/texture_render_target.cpp
        ue/textures/texture_render_target.hpp
        ue/textures/texture_render_target_2_d.cpp
        ue/textures/texture_render_target_2_d.hpp

        # Satisfactory Specifics
        satisfactory/fg_recipe.cpp
        satisfactory/fg_recipe.hpp
        satisfactory/items/fg_item_descriptor.cpp
        satisfactory/items/fg_item_descriptor.hpp
        satisfactory/items/fg_item_descriptor_biomass.cpp
        satisfactory/items/fg_item_descriptor_biomass.hpp
        satisfactory/fg_resource_descriptor.cpp
        satisfactory/fg_resource_descriptor.hpp
        satisfactory/building/fg_building_descriptor.cpp
        satisfactory/building/fg_building_descriptor.hpp
        satisfactory/fg_schematic.cpp
        satisfactory/fg_schematic.hpp
)

target_link_libraries(SAA_Assets
        PRIVATE
        SAA_Structs
        SAA_Parser
        SAA_Util
        SAA_Lib
        SAA_Exporter
        SAA_Localization
        StructuredChaos
)

target_include_directories(SAA_Assets
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)