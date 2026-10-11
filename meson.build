project(
  'SatisfactoryAssetAssembler',
  'cpp',
  version: '0.1.0',
  default_options: ['cpp_std=c++23', 'buildtype=release', 'b_ndebug=if-release'],
  meson_version: '>= 1.1.0',
)

# Sanitizer builds are selected with the built-in option, for example
#   meson setup build/asan --buildtype=debug -Db_sanitize=address,undefined
# LTO likewise:
#   meson setup build/release --buildtype=release -Db_lto=true

cpp = meson.get_compiler('cpp')
is_msvc_like = cpp.get_argument_syntax() == 'msvc'
is_optimized = get_option('optimization') not in ['0', 'g']

saa_args = []
if is_msvc_like
  saa_args += '/arch:AVX2'
else
  saa_args += '-mcx16'
  if meson.is_cross_build()
    saa_args += '-march=x86-64-v2'
  elif get_option('native_arch')
    saa_args += '-march=native'
  endif
  # UBSan's vptr check needs RTTI, so only optimized builds go without.
  if is_optimized
    saa_args += '-fno-rtti'
  endif
endif
add_project_arguments(cpp.get_supported_arguments(saa_args), language: 'cpp')
# windows.h defines min/max as macros and pulls in Winsock, whose PF_MAX collides with EPixelFormat.
if host_machine.system() == 'windows'
  add_project_arguments('-DNOMINMAX', '-DWIN32_LEAN_AND_MEAN', language: 'cpp')
endif
if get_option('track_properties')
  add_project_arguments('-DSAA_TRACK_PROPERTIES', language: 'cpp')
endif

subdir('external')
subdir('src')

if not meson.is_subproject() and host_machine.system() != 'windows'
  run_command(
    'ln',
    '-sf', meson.project_build_root() / 'compile_commands.json',
    meson.project_source_root() / 'compile_commands.json',
    check: false,
  )
endif
