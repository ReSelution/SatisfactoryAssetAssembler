add_library(SAP_Structs STATIC)

target_sources(SAP_Structs
        PUBLIC
        functions.hpp
        version.hpp

        core/name.hpp
        core/game.hpp
        core/package.hpp
        core/hash.hpp
        core/flags.hpp
        core/constants.hpp
        core/f_guid.hpp

        core/properties/f_property_type_name_node.hpp
        core/properties/f_property_tag.hpp

        io/f_io_hash.hpp
        io/f_io_chunk_id.hpp
        io/f_on_demand_toc_entry.hpp
        io/f_io_store_chunk_block_meta.hpp
        io/f_io_file_index_entry.hpp
        io/f_io_container_id.hpp
        io/f_io_container_header.hpp
        io/f_file_package_store_entry.hpp
        io/f_io_container_header_soft_package_refenrenes.hpp
        io/f_io_container_header_localized_package.hpp
        io/f_io_container_header_package_redirect.hpp
        io/f_io_store_toc_header.hpp
        io/f_io_toc_container_id.hpp
        io/f_io_offset_and_lenght.hpp
        io/f_io_store_toc_Compressed_block_entry.hpp
        io/f_io_directory_index_enty.hpp


        io/packages/f_bulk_data_cooked_index.hpp
        io/packages/f_bulk_data_map_entry.hpp
        io/packages/f_export_bundle_entry.hpp
        io/packages/f_export_bundle_header.hpp
        io/packages/f_zen_package_cell_offsets.hpp
        io/packages/f_custom_version.hpp
        io/packages/f_zen_package_versioning_info.hpp
        io/packages/f_object_import.hpp
        io/packages/f_object_export.hpp
        io/packages/f_file_summary.hpp
        io/packages/f_zen_package_summary.hpp
        io/packages/f_export_map_entry.hpp

        usmap/f_usmap_property_data.hpp
        usmap/f_usmap_schema.hpp
        usmap/enums.hpp
        usmap/f_usmap_mapping_data.hpp

        assets/textures/pixel_format.hpp
        assets/textures/texture_compression_settings.hpp
        assets/textures/texture_group.hpp

        core/properties/f_property_tag_data.hpp
)

target_sources(SAP_Structs
        PRIVATE
        core/package.cpp
)

target_include_directories(SAP_Structs
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        PRIVATE
        external/cityhash
)

target_link_libraries(SAP_Structs
        PRIVATE
        CITY_HASH
)