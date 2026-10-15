# Dependency
sc_dep = dependency(
  'structured_chaos',
  fallback: ['StructuredChaos', 'sc_dep'],
  default_options: [
    'enable_chaos_stats=@0@'.format(get_option('enable_chaos_stats')),
    'dump_magpie=@0@'.format(get_option('dump_magpie')),
  ],
)

# byte_cursor.cpp uses it directly, sc_dep no longer passes it on. Same subproject and options as in StructuredChaos.
simdutf_dep = dependency(
  'simdutf',
  fallback: ['simdutf', 'simdutf_dep'],
  default_options: ['default_library=static'],
)

# The JSON parser of fastgltf. Resolved here first so that it is built statically as well: fastgltf only passes
# its own default_options on, and as a DLL simdjson does not export every symbol fastgltf needs.
dependency(
  'simdjson',
  fallback: ['simdjson', 'simdjson_dep'],
  default_options: ['default_library=static'],
)

fastgltf_dep = dependency(
  'fastgltf',
  fallback: ['fastgltf', 'fastgltf_dep'],
  default_options: ['default_library=static'],
)

# Header-only, pinned to the commits the git submodules pointed to.
stb_dep = dependency('stb', fallback: ['stb', 'stb_dep'])
bcdec_dep = dependency('bcdec', fallback: ['bcdec', 'bcdec_dep'])

# Command line of the executable only, kept out of all_deps so the library does not see it.
cli11_dep = dependency('CLI11', fallback: ['cli11', 'CLI11_dep'])

# Vendored in subprojects/cityhash, there is no wrap for it.
cityhash_dep = dependency('cityhash', fallback: ['cityhash', 'cityhash_dep'])

# dlopen() for the Oodle library
dl_dep = cpp.find_library('dl', required: false)

all_deps = [
  sc_dep,
  simdutf_dep,
  fastgltf_dep,
  cityhash_dep,
  stb_dep,
  bcdec_dep,
  dependency('threads'),
  dl_dep,
]
