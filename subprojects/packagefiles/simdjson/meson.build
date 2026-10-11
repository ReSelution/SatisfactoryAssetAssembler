project(
  'simdjson',
  'cpp',
  version: '3.12.3',
  license: 'Apache-2.0 OR MIT',
  default_options: ['cpp_std=c++17'],
  meson_version: '>=1.1.0',
)

compiler = meson.get_compiler('cpp')
is_windows = host_machine.system() == 'windows'

# Defines that consumers of the headers must see as well
simdjson_public_args = []
# Defines and flags for the library itself only
simdjson_private_args = []
simdjson_deps = []

if get_option('enable_threads')
  simdjson_public_args += '-DSIMDJSON_THREADS_ENABLED=1'
  simdjson_deps += dependency('threads')
endif

if not get_option('exceptions')
  simdjson_public_args += '-DSIMDJSON_EXCEPTIONS=0'
endif

if get_option('disable_deprecated_api')
  simdjson_public_args += '-DSIMDJSON_DISABLE_DEPRECATED_API=1'
endif

if get_option('development_checks')
  simdjson_public_args += '-DSIMDJSON_DEVELOPMENT_CHECKS=1'
endif

if get_option('verbose_logging')
  simdjson_private_args += '-DSIMDJSON_VERBOSE_LOGGING=1'
endif

foreach impl : [
  'icelake',
  'haswell',
  'westmere',
  'arm64',
  'ppc64',
  'lsx',
  'lasx',
  'fallback',
]
  choice = get_option('impl_' + impl)
  if choice != 'auto'
    simdjson_public_args += '-DSIMDJSON_IMPLEMENTATION_@0@=@1@'.format(
      impl.to_upper(),
      choice == 'enabled' ? 1 : 0,
    )
  endif
endforeach

builtin_impl = get_option('builtin_implementation')
if builtin_impl != ''
  simdjson_public_args += '-DSIMDJSON_BUILTIN_IMPLEMENTATION=' + builtin_impl
endif

# Workaround for GCC's poor code generation for unaligned AVX loads/stores
if compiler.get_id() == 'gcc' and host_machine.cpu_family() in ['x86', 'x86_64']
  simdjson_private_args += [
    '-mno-avx256-split-unaligned-load',
    '-mno-avx256-split-unaligned-store',
  ]
endif

simdjson_dll_build_args = []
simdjson_dll_use_args = []
if is_windows and get_option('default_library') == 'shared'
  simdjson_dll_build_args += '-DSIMDJSON_BUILDING_WINDOWS_DYNAMIC_LIBRARY=1'
  simdjson_dll_use_args += '-DSIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY=1'
endif

# The amalgamation is checked in upstream: one source file, one header.
simdjson_inc = include_directories('singleheader')

simdjson_lib = library(
  'simdjson',
  files('singleheader/simdjson.cpp'),
  include_directories: simdjson_inc,
  cpp_args: simdjson_public_args + simdjson_private_args + simdjson_dll_build_args,
  dependencies: simdjson_deps,
  pic: true,
)

simdjson_dep = declare_dependency(
  include_directories: simdjson_inc,
  compile_args: simdjson_public_args + simdjson_dll_use_args,
  link_with: simdjson_lib,
  dependencies: simdjson_deps,
  version: meson.project_version(),
)

meson.override_dependency('simdjson', simdjson_dep)
