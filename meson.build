project('kiwix-tools', 'cpp')

compiler = meson.get_compiler('cpp')

static_linkage = get_option('static-linkage')
if static_linkage
  add_global_link_arguments('-static-libstdc++', '--static', language:'cpp')
endif

thread_dep = dependency('threads')
kiwixlib_dep = dependency('kiwix', static:static_linkage)
microhttpd_dep = dependency('libmicrohttpd', static:static_linkage)
z_dep = dependency('zlib', static:static_linkage)

# Idealy we should not have more dependency, however :
# We should declare we use ctpp2 in kiwixlib in the pkg-config file.
# But there is no pkg-config file for ctpp2. Once one exists, no need to
# declare the dependency here
ctpp2_prefix_install = get_option('ctpp2-install-prefix')
find_library_in_compiler = meson.version().version_compare('>=0.31.0')
if ctpp2_prefix_install == ''
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp')
    error('ctpp2/CTPP2Logger.hppnot found')
  endif
  if find_library_in_compiler
    ctpp2_lib = compiler.find_library('ctpp2')
  else
    ctpp2_lib = find_library('ctpp2')
  endif
  link_args = ['-lctpp2']
  if meson.is_cross_build()
    if host_machine.system() == 'windows'
      link_args += ['-liconv']
    endif
  endif
  ctpp2_dep = declare_dependency(link_args:link_args)
else
  if not find_library_in_compiler
    error('For custom ctpp2_prefix_install you need a meson version >=0.31.0')
  endif
  ctpp2_include_path = ctpp2_prefix_install + '/include'
  ctpp2_include_args = ['-I'+ctpp2_include_path]
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp', args:ctpp2_include_args)
    error('ctpp2/CTPP2Logger.hpp not found')
  endif
  ctpp2_include_path = include_directories(ctpp2_include_path, is_system:true)
  ctpp2_lib_path = ctpp2_prefix_install+'/lib'
  ctpp2_lib = compiler.find_library('ctpp2', dirs:ctpp2_lib_path)
  link_args = ['-L'+ctpp2_lib_path, '-lctpp2']
  if meson.is_cross_build()
    if host_machine.system() == 'windows'
      link_args += ['-liconv']
    endif
  endif
  ctpp2_dep = declare_dependency(include_directories:ctpp2_include_path, link_args:link_args)
endif

all_deps = [thread_dep, kiwixlib_dep, microhttpd_dep, ctpp2_dep, z_dep]

#subdir('include')
subdir('static')
subdir('src')
