# ==============================================================================
# SAA_Structs - The Core Data Structures
# ==============================================================================
add_library(SAA_Structs STATIC
        assets/satisfactory/ItemAmount.hpp
        core/EClassSerializationControlExtension.hpp
)

# ------------------------------------------------------------------------------
# PUBLIC INTERFACE (Header Files)
# ------------------------------------------------------------------------------
target_sources(SAA_Structs
        PUBLIC
        # --- Core & Math ---
        core/math/f_vector.hpp
        core/math/f_box.hpp
        core/math/f_quat.hpp
        core/f_linear_color.hpp
        core/f_color.hpp
        core/f_guid.hpp
        core/f_text.hpp
        core/f_box_sphere_bounds.hpp
        core/half.hpp
        core/name.hpp
        core/hash.hpp
        core/flags.hpp
        core/constants.hpp
        core/package.hpp
        core/per_quality_level_props.hpp
        version.hpp
        functions.hpp


        # --- Curves ---
        core/curves/rich_curve.hpp
        core/curves/f_compressed_rich_curve.hpp

        # --- IO & Zen Loader (UE5) ---
        io/f_io_hash.hpp
        io/f_io_chunk_id.hpp
        io/f_io_container_id.hpp
        io/f_io_container_header.hpp
        io/f_io_container_header_soft_package_refenrenes.hpp
        io/f_io_container_header_localized_package.hpp
        io/f_io_container_header_package_redirect.hpp
        io/f_io_store_toc_header.hpp
        io/f_io_toc_container_id.hpp
        io/f_io_offset_and_lenght.hpp
        io/f_io_store_toc_Compressed_block_entry.hpp
        io/f_io_store_chunk_block_meta.hpp
        io/f_io_directory_index_enty.hpp
        io/f_io_file_index_entry.hpp
        io/f_on_demand_toc_entry.hpp
        io/f_file_package_store_entry.hpp

        # --- Packages & Bulk Data ---
        io/packages/f_file_summary.hpp
        io/packages/f_zen_package_summary.hpp
        io/packages/f_zen_package_cell_offsets.hpp
        io/packages/f_zen_package_versioning_info.hpp
        io/packages/f_byte_bulk_data.hpp
        io/packages/f_byte_bulk_data_header.hpp
        io/packages/f_bulk_data_cooked_index.hpp
        io/packages/f_bulk_data_map_entry.hpp
        io/packages/f_export_map_entry.hpp
        io/packages/f_export_bundle_header.hpp
        io/packages/f_export_bundle_entry.hpp
        io/packages/f_object_import.hpp
        io/packages/f_object_export.hpp
        io/packages/f_custom_version.hpp

        # --- Pak Files (Legacy/Classic) ---
        pak/f_pak_info.hpp
        pak/e_pak_file_version.hpp
        pak/f_pak_entry.hpp

        # --- Mappings (USMAP) ---
        usmap/f_usmap_property_data.hpp
        usmap/f_usmap_schema.hpp
        usmap/f_usmap_mapping_data.hpp
        usmap/enums.hpp

        # --- Nanite ---
        assets/nanite/nanite_constants.hpp
        assets/nanite/f_nanit_resources.hpp
        assets/nanite/f_page_streaming_state.hpp
        assets/nanite/f_hierarchy_node_slice.hpp
        assets/nanite/f_packed_hierachy_node.hpp
        assets/material/f_material_override_nanite.hpp

        # --- Static Mesh & Rendering ---
        assets/static_mesh/f_static_mesh_render_data.hpp
        assets/static_mesh/f_static_mesh_LOD_resources.hpp
        assets/static_mesh/f_static_mesh_section.hpp
        assets/static_mesh/f_static_mesh_vertex_buffer.hpp
        assets/static_mesh/f_static_mesh_UV_item.hpp
        assets/static_mesh/f_static_raw_index_buffer.hpp
        assets/static_mesh/f_static_material.hpp
        assets/static_mesh/f_card_represantation_data.hpp
        assets/static_mesh/distance_field_atlas.hpp
        assets/meshes/f_postion_vertex_Buffer.hpp
        assets/meshes/f_color_vertex_buffer.hpp
        assets/meshes/f_mesh_UV_float.hpp
        assets/meshes/f_mesh_UV_channel_info.hpp
        assets/meshes/f_weighted_random_sampler.hpp
        render_core/f_packed_normal.hpp

        # --- Materials ---
        assets/material/e_blend_mode.hpp
        assets/material/e_translucency_lighting_mode.hpp
        assets/material/e_material_shading_model.hpp
        assets/material/e_material_parameter_association.hpp
        assets/material/f_material_instance_base_property_overrides.hpp
        assets/material/f_material_parameter_info.hpp
        assets/material/f_static_parameter_set.hpp
        assets/material/f_static_parameter_base.hpp
        assets/material/f_static_switch_parameter.hpp
        assets/material/f_static_component_mask_parameter.hpp
        assets/material/f_static_terrain_layer_weight_parameter.hpp
        assets/material/f_static_material_layers_parameter.hpp
        assets/material/f_scalar_parameter_value.hpp
        assets/material/f_texture_parameter_value.hpp
        assets/material/f_vector_parameter_value.hpp

        # --- Textures ---
        assets/textures/pixel_format.hpp
        assets/textures/texture_group.hpp
        assets/textures/texture_compression_settings.hpp
        assets/textures/f_texture_2d_mip_map.hpp
        assets/textures/f_opt_texture_platfrom_data.hpp
        assets/textures/f_virtual_texture_built_data.hpp
        assets/textures/f_virtual_texturte_data_chunk.hpp
        assets/textures/f_virtual_texture_tile_offset_data.hpp

        # --- Localization ---
        localization/f_text_localization_resource_string.hpp
        localization/f_text_key.hpp
        localization/f_entry.hpp
)

# ------------------------------------------------------------------------------
# PRIVATE IMPLEMENTATION
# ------------------------------------------------------------------------------
target_sources(SAA_Structs
        PRIVATE
        core/package.cpp

)

# ------------------------------------------------------------------------------
# DEPENDENCIES & INCLUDES
# ------------------------------------------------------------------------------
target_include_directories(SAA_Structs
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        external/cityhash
)

target_link_libraries(SAA_Structs
        PUBLIC
        SAA_Util
        StructuredChaos
        SAA_Assets
        SAA_Parser
        PRIVATE
        CITY_HASH
)