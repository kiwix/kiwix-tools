
sources = ['kiwix-serve.cpp', 'request_context.cpp']
sources += server_resources

executable('kiwix-serve', sources,
  dependencies:all_deps,
  install:true,
  install_rpath: join_paths(get_option('prefix'), get_option('libdir')))
