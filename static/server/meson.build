
server_resources = custom_target('resources',
  input: 'resources_list.txt',
  output: ['server-resources.cpp', 'server-resources.h'],
  command:[res_compiler,
           '--cxxfile', '@OUTPUT0@',
           '--hfile', '@OUTPUT1@', '@INPUT@', '@OUTDIR@'],
  build_always: true
)
  
