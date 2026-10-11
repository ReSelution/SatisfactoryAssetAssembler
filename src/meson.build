saa_inc = include_directories(
  'assets',
  'crawler',
  'decompress',
  'exporter',
  'localizations',
  'parser',
  'provider',
  'reader',
  'structs',
  'util',
  'vfs',
)

saa_sources = files(
  'assets/asset_object.cpp',
  'assets/object_factory.cpp',
  'assets/satisfactory/building/fg_building_base.cpp',
  'assets/satisfactory/building/fg_building_descriptor.cpp',
  'assets/satisfactory/fg_descriptor_base.cpp',
  'assets/satisfactory/fg_equipment_descriptor.cpp',
  'assets/satisfactory/fg_factory.cpp',
  'assets/satisfactory/fg_recipe.cpp',
  'assets/satisfactory/fg_resource_descriptor.cpp',
  'assets/satisfactory/fg_schematic.cpp',
  'assets/satisfactory/items/fg_item_base.cpp',
  'assets/satisfactory/items/fg_item_descriptor_biomass.cpp',
  'assets/satisfactory/items/fg_item_descriptor.cpp',
  'assets/ue/controlRig/rig_hierarchy.cpp',
  'assets/ue/engine/blueprint_generated_class.cpp',
  'assets/ue/material/material.cpp',
  'assets/ue/material/material_instance_constant.cpp',
  'assets/ue/material/material_instance.cpp',
  'assets/ue/material/material_interface.cpp',
  'assets/ue/material/unreal_material.cpp',
  'assets/ue/object/class.cpp',
  'assets/ue/object/field.cpp',
  'assets/ue/object/struct.cpp',
  'assets/ue/properties/property_holder.cpp',
  'assets/ue/properties/property_parser_1009.cpp',
  'assets/ue/properties/property_parser_1017.cpp',
  'assets/ue/properties/soft_object.cpp',
  'assets/ue/rigVM/rig_vm.cpp',
  'assets/ue/static_mesh/static_mesh.cpp',
  'assets/ue/static_mesh/static_mesh_socket.cpp',
  'assets/ue/textures/texture_2d_array.cpp',
  'assets/ue/textures/texture_2d.cpp',
  'assets/ue/textures/texture.cpp',
  'assets/ue/textures/texture_render_target_2_d.cpp',
  'assets/ue/textures/texture_render_target.cpp',
  'crawler/crawler.cpp',
  'decompress/Decompressor.cpp',
  'decompress/oodle_loader.cpp',
  'exporter/exporter.cpp',
  'exporter/fg/fg_exporter.cpp',
  'exporter/material/material_exporter.cpp',
  'exporter/meshes/mesh_exporter.cpp',
  'exporter/texture/texture_decoder.cpp',
  'exporter/texture/texture_exporter.cpp',
  'localizations/loc_manager.cpp',
  'localizations/loc_meta_parser.cpp',
  'localizations/loc_res_parser.cpp',
  'parser/AssetFileParser.cpp',
  'parser/AssetParser.cpp',
  'parser/uasset_parser.cpp',
  'provider/games/satisfactory/SatisfactoryDataProvider.cpp',
  'reader/file_reader.cpp',
  'reader/io_global_data.cpp',
  'reader/IOStoreReader.cpp',
  'reader/PAKReader.cpp',
  'reader/Reader.cpp',
  'reader/UsmapReader.cpp',
  'structs/core/hash.cpp',
  'structs/core/name.cpp',
  'structs/core/package.cpp',
  'util/byte_cursor.cpp',
  'util/PropertyTracker.cpp',
  'vfs/game_file.cpp',
  'vfs/vfs.cpp',
  'vfs/vfs_entry.cpp',
)

# The library only needs the compile side of its dependencies. sc_dep carries mi_new_delete.cpp as a source on
# Windows; taken along here it would end up in the library and, through saa_dep, a second time in the executable.
saa_lib_deps = []
foreach dep : all_deps
  saa_lib_deps += dep.partial_dependency(compile_args: true, includes: true)
endforeach

saa_lib = static_library(
  'SatisfactoryAssetAssembler',
  saa_sources,
  include_directories: saa_inc,
  dependencies: saa_lib_deps,
)

# The asset classes register themselves with the object factory from static
# initializers, which the linker would drop from a plain static library.
saa_dep = declare_dependency(
  link_whole: saa_lib,
  include_directories: saa_inc,
  dependencies: all_deps,
)

if not meson.is_subproject()
  executable(
    'AssetRipperTest',
    'main.cpp',
    dependencies: saa_dep,
  )
endif
