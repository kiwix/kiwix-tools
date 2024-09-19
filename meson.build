project('kiwix-tools', 'cpp',
  version : '3.8.0',
  license : 'GPL',
  default_options: ['c_std=c11', 'cpp_std=c++17', 'werror=true'])

compiler = meson.get_compiler('cpp')

add_global_arguments('-DKIWIX_TOOLS_VERSION="@0@"'.format(meson.project_version()), language : 'cpp')

if host_machine.system() == 'windows'
  add_project_arguments('-DNOMINMAX', language: 'cpp')
endif

static_linkage = get_option('static-linkage')
if static_linkage
  # Static build is not supported on MacOS
  if host_machine.system() != 'darwin'
    add_global_link_arguments('-static-libstdc++', '--static', language:'cpp')
  endif
endif

thread_dep = dependency('threads')
libzim_dep = dependency('libzim', version:['>=9.0.0', '<10.0.0'], static:static_linkage)
libkiwix_dep = dependency('libkiwix', version:['>=14.0.0', '<15.0.0'], static:static_linkage)
libdocopt_dep = dependency('docopt', static:static_linkage)

all_deps = [thread_dep, libkiwix_dep, libzim_dep, libdocopt_dep]

if static_linkage
  librt = compiler.find_library('rt', required:false)
  if librt.found()
    all_deps += librt
  endif
endif

subdir('src')
if get_option('doc')
  subdir('docs')
endif
