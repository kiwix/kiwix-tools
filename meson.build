project('kiwix-tools', 'cpp',
  version : '2.1.0',
  license : 'GPL',
  default_options: ['c_std=c11', 'cpp_std=c++11', 'werror=true'])

compiler = meson.get_compiler('cpp')

static_linkage = get_option('static-linkage')
if static_linkage
  add_global_link_arguments('-static-libstdc++', '--static', language:'cpp')
endif

thread_dep = dependency('threads')
kiwixlib_dep = dependency('kiwix', version:'>=5.2.0', static:static_linkage)
microhttpd_dep = dependency('libmicrohttpd', static:static_linkage)
z_dep = dependency('zlib', static:static_linkage)

all_deps = [thread_dep, kiwixlib_dep, microhttpd_dep, z_dep]

if static_linkage
  librt = compiler.find_library('rt', required:false)
  if librt.found()
    all_deps += librt
  endif
endif

#subdir('include')
subdir('static')
subdir('src')
