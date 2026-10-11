project(
  'stb',
  'c',
  # Upstream has no releases, this is the commit of the wrap.
  version: 'f1c79c0',
  license: 'MIT OR Unlicense',
  meson_version: '>=1.1.0',
)

# Header-only. Each header is its own library and compiled by the one source
# file that defines its *_IMPLEMENTATION macro.
stb_dep = declare_dependency(include_directories: include_directories('.'))

meson.override_dependency('stb', stb_dep)
