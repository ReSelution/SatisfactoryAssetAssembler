project(
  'cityhash',
  'cpp',
  version: '1.1.1',
  license: 'MIT',
  default_options: ['cpp_std=c++17'],
  meson_version: '>=1.1.0',
)

# Vendored and changed: city_inline.hpp is not part of upstream.
cityhash_inc = include_directories('.')

cityhash_lib = static_library(
  'cityhash',
  files('city.cc'),
  include_directories: cityhash_inc,
  pic: true,
)

cityhash_dep = declare_dependency(
  include_directories: cityhash_inc,
  link_with: cityhash_lib,
  version: meson.project_version(),
)

meson.override_dependency('cityhash', cityhash_dep)
