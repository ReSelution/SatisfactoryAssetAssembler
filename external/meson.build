# Dependency
sc_dep = dependency(
  'structured_chaos',
  fallback: ['StructuredChaos', 'sc_dep'],
  default_options: [
    'enable_chaos_stats=@0@'.format(get_option('enable_chaos_stats')),
    'dump_magpie=@0@'.format(get_option('dump_magpie')),
  ],
)

# Pulls in simdjson itself, which is its JSON parser.
fastgltf_dep = dependency(
  'fastgltf',
  fallback: ['fastgltf', 'fastgltf_dep'],
  default_options: ['default_library=static'],
)

# Vendored in the tree. stb and bcdec are header-only and included as
# "stb/..." and "bcdec/...", so this directory is their include root.
external_inc = include_directories('.')

cityhash_inc = include_directories('cityhash')
cityhash_lib = static_library(
  'cityhash',
  'cityhash/city.cc',
  include_directories: cityhash_inc,
  override_options: ['cpp_std=c++17'],
)
cityhash_dep = declare_dependency(include_directories: cityhash_inc, link_with: cityhash_lib)

# dlopen() for the Oodle library
dl_dep = cpp.find_library('dl', required: false)

all_deps = [
  sc_dep,
  fastgltf_dep,
  cityhash_dep,
  dependency('threads'),
  dl_dep,
]
