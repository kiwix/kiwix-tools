project('kiwix-tools', 'cpp',
  version : '0.3.0',
  license : 'GPL',
  default_options: ['c_std=c11', 'cpp_std=c++11'])

compiler = meson.get_compiler('cpp')

static_linkage = get_option('static-linkage')
if static_linkage
  add_global_link_arguments('-static-libstdc++', '--static', language:'cpp')
endif

thread_dep = dependency('threads')
kiwixlib_dep = dependency('kiwix', version:'>=1.0.0', static:static_linkage)
microhttpd_dep = dependency('libmicrohttpd', static:static_linkage)
z_dep = dependency('zlib', static:static_linkage)

# This is a temporary workaround until xapian fix it's xapian-core.pc
# For a correct dependency checking we should test if uuid is really installed
# but for a workaround, we assume that in xapian is installed, uuid also.
if meson.is_cross_build() and host_machine.system() == 'windows'
    # xapian doesn't use uuid on windows.
    uuid_dep = declare_dependency()
else
    xapian_dep = dependency('xapian-core', required : false)
    if xapian_dep.found()
        uuid_dep = declare_dependency(link_args:['-luuid', '-lrt'])
    else
        uuid_dep = declare_dependency()
    endif
endif

all_deps = [thread_dep, kiwixlib_dep, microhttpd_dep, z_dep, uuid_dep]

#subdir('include')
subdir('static')
subdir('src')
