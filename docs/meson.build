
sphinx = find_program('sphinx-build', native:true)

sphinx_target = run_target('doc',
    command: [sphinx, '-bhtml',
              meson.current_source_dir(),
              meson.current_build_dir()])
