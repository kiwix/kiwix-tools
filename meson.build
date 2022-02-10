project('kiwix-tools', 'cpp',
  version : '3.2.0',
  license : 'GPL',
  default_options: ['c_std=c11', 'cpp_std=c++11', 'werror=true'])

compiler = meson.get_compiler('cpp')

add_global_arguments('-DKIWIX_TOOLS_VERSION="@0@"'.format(meson.project_version()), language : 'cpp')

static_linkage = get_option('static-linkage')
if static_linkage
  add_global_link_arguments('-static-libstdc++', '--static', language:'cpp')
endif

thread_dep = dependency('threads')
kiwixlib_dep = dependency('kiwix', version:'>=10.0.0', static:static_linkage)
libzim_dep = dependency('libzim', version:'>=7.2.0', static:static_linkage)

all_deps = [thread_dep, kiwixlib_dep, libzim_dep]

if static_linkage
  librt = compiler.find_library('rt', required:false)
  if librt.found()
    all_deps += librt
  endif
endif

subdir('src')
