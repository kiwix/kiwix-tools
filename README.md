kiwix-tools
===========

kiwix-tools contains a set of tools to interact with kiwix and zim files.



Build kiwix-tools
-----------------


Most of the compilation steps (including dependencies download and compilation)
are handle by [kiwix-build](https://github.com/kiwix/kiwix-build) script.
If you don't have any special need, we recommand you to use kiwix-build
instead of doing all the steps yourself.

Dependencies:

You'll need the following dependencies to build libkiwix:

* [kiwix-lib](https://github.com/kiwix/kiwix-lib) (and its dependencies)
* [libmicrohttpd](http://www.openzim.org/wiki/Zimlib)

As we use meson to build kiwix-tools, you will need the common meson tools:
* [meson](http://mesonbuild.com/) >= 0.34
* ninja
* pkg-config

To build:

```
$ cd kiwix-lib
$ meson . build
$ cd build
$ ninja
$ ninja install
```

By default, it will compile dynamic linked binaries.
If you want statically linked binaries, you can add `--default-library=static`
option to the meson command.

Licensed as GPLv3 or later, see COPYING for more details.
