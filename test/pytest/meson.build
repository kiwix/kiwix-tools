py3_dep = dependency('python3', required : false)

if py3_dep.found()
  py3_mod = import('python3')
  py3 = py3_mod.find_python()

  has_pytest = (run_command(py3, ['-m', 'pytest', '--version']).returncode() == 0)

  if has_pytest
    test('pytest',
      py3,
      args: ['-m', 'pytest'],
      workdir: meson.current_source_dir(),
      env: ['KIWIX_SERVE_EXE_PATH=' + kiwix_serve_target.full_path()]
    )
  endif
endif
