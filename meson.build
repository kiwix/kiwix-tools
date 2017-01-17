project('kiwix-tools', 'cpp')

compiler = meson.get_compiler('cpp')

thread_dep = dependency('threads')
kiwixlib_dep = dependency('kiwix')
microhttpd_dep = dependency('libmicrohttpd')
z_dep = dependency('zlib')

# Idealy we should not have more dependency, however :
# We should declare we use ctpp2 in kiwixlib in the pkg-config file.
# But there is no pkg-config file for ctpp2. Once one exists, no need to
# declare the dependency here
ctpp2_prefix_install = get_option('ctpp2-install-prefix')
if get_option('default_library') == 'static'
    libname = 'ctpp2-st'
else
    libname = 'ctpp2'
endif
if ctpp2_prefix_install == ''
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp')
    error('ctpp2/CTPP2Logger.hppnot found')
  endif
  ctpp2_lib = compiler.find_library(libname)
  link_args = ['-l'+libname]
  if meson.is_cross_build()
    if host_machine.system() == 'windows'
      link_args += ['-liconv']
    endif
  endif
  ctpp2_dep = declare_dependency(link_args:link_args)
else
  ctpp2_include_path = ctpp2_prefix_install + '/include'
  ctpp2_include_args = ['-I'+ctpp2_include_path]
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp', args:ctpp2_include_args)
    error('ctpp2/CTPP2Logger.hpp not found')
  endif
  ctpp2_include_path = include_directories(ctpp2_include_path, is_system:true)
  ctpp2_lib_path = ctpp2_prefix_install+'/lib'
  ctpp2_lib = compiler.find_library(libname, dirs:ctpp2_lib_path)
  link_args = ['-L'+ctpp2_lib_path, '-l'+libname]
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
