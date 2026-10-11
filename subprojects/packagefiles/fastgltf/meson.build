project(
  'fastgltf',
  'cpp',
  version: '0.9.0',
  license: 'MIT',
  default_options: ['cpp_std=c++17'],
  meson_version: '>=1.1.0',
)

compiler = meson.get_compiler('cpp')
compiler_id = compiler.get_id()
is_msvc_like = compiler.get_argument_syntax() == 'msvc'

# Defines that consumers of the headers must see as well. Upstream always sets
# them, to 0 or 1.
fastgltf_public_args = []
# Defines and flags for the library itself only
fastgltf_private_args = []
fastgltf_overrides = []

enable_khr_physics_rigid_bodies = get_option('enable_khr_physics_rigid_bodies')
# The rigid bodies extension builds on the implicit shapes one
enable_khr_implicit_shapes = get_option('enable_khr_implicit_shapes') or enable_khr_physics_rigid_bodies

foreach name, enabled : {
  'USE_CUSTOM_SMALLVECTOR': get_option('use_custom_smallvector'),
  'ENABLE_DEPRECATED_EXT': get_option('enable_deprecated_ext'),
  'DISABLE_CUSTOM_MEMORY_POOL': get_option('disable_custom_memory_pool'),
  'USE_64BIT_FLOAT': get_option('use_64bit_float'),
  'ENABLE_KHR_IMPLICIT_SHAPES': enable_khr_implicit_shapes,
  'ENABLE_KHR_PHYSICS_RIGID_BODIES': enable_khr_physics_rigid_bodies,
}
  fastgltf_public_args += '-DFASTGLTF_@0@=@1@'.format(name, enabled ? 1 : 0)
endforeach

if get_option('compile_as_cpp20')
  fastgltf_overrides += 'cpp_std=c++20'
endif

if is_msvc_like
  fastgltf_private_args += ['/EHsc', '/utf-8']
else
  fastgltf_private_args += '-Wno-unknown-pragmas'
  if compiler_id == 'gcc'
    fastgltf_private_args += ['-flax-vector-conversions', '-fpermissive']
  endif
endif

# Upstream inlines in debug builds too, the parser is unusably slow otherwise
if get_option('optimization') == '0'
  fastgltf_private_args += is_msvc_like ? '/Ob2' : '-finline-functions'
endif

# Upstream downloads the simdjson amalgamation into deps/ when there is no
# simdjson target. Here it always comes from the subproject of the same version.
simdjson_dep = dependency('simdjson', version: '>=3.12.3')

fastgltf_inc = include_directories('include')

fastgltf_lib = library(
  'fastgltf',
  files('src/base64.cpp', 'src/fastgltf.cpp', 'src/io.cpp'),
  include_directories: fastgltf_inc,
  cpp_args: fastgltf_public_args + fastgltf_private_args,
  dependencies: simdjson_dep,
  override_options: fastgltf_overrides,
  version: meson.project_version(),
  pic: true,
)

fastgltf_dep = declare_dependency(
  include_directories: fastgltf_inc,
  compile_args: fastgltf_public_args,
  link_with: fastgltf_lib,
  version: meson.project_version(),
)

meson.override_dependency('fastgltf', fastgltf_dep)
