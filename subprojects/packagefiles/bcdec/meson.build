project(
  'bcdec',
  'c',
  # Upstream has no releases, this is the commit of the wrap.
  version: '93628fe',
  license: 'MIT OR Unlicense',
  meson_version: '>=1.1.0',
)

# Header-only, compiled by the one source file that defines
# BCDEC_IMPLEMENTATION.
bcdec_dep = declare_dependency(include_directories: include_directories('.'))

meson.override_dependency('bcdec', bcdec_dep)
