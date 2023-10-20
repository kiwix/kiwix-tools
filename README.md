Kiwix tools
===========

The Kiwix tools is a collection of [Kiwix](https://kiwix.org) related
command line tools:
* kiwix-manage: Manage XML based library of ZIM files
* kiwix-search: Full text search in ZIM files
* kiwix-serve: HTTP daemon serving ZIM files

[![latest release](https://img.shields.io/github/v/tag/kiwix/kiwix-tools?label=latest%20release&sort=semver)](https://download.kiwix.org/release/kiwix-tools/)
[![Repositories](https://img.shields.io/repology/repositories/kiwix-tools?label=repositories)](https://github.com/kiwix/kiwix-tools/wiki/Repology)
[![Docker](https://ghcr-badge.deta.dev/kiwix/kiwix-tools/latest_tag?label=docker)](https://ghcr.io/kiwix/kiwix-tools)
[![Docker](https://ghcr-badge.deta.dev/kiwix/kiwix-tools/latest_tag?label=docker%20(kiwix-serve))](https://ghcr.io/kiwix/kiwix-tools)
[![Sandstorm](https://img.shields.io/badge/Sandstorm-kiwix-blue)](https://apps.sandstorm.io/app/5uh349d0kky2zp5whrh2znahn27gwha876xze3864n0fu9e5220h)
[![Build Status](https://github.com/kiwix/kiwix-tools/workflows/CI/badge.svg?query=branch%3Amain)](https://github.com/kiwix/kiwix-tools/actions?query=branch%3Amain)
[![Doc](https://readthedocs.org/projects/kiwix-tools/badge/?style=flat)](https://kiwix-tools.readthedocs.org/en/latest/?badge=latest)
[![CodeFactor](https://www.codefactor.io/repository/github/kiwix/kiwix-tools/badge)](https://www.codefactor.io/repository/github/kiwix/kiwix-tools)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Kiwix tools compilation itself, we recommend to have a look
to [kiwix-build](https://github.com/kiwix/kiwix-build).

Preamble
--------

Although the Kiwix tools can be compiled/cross-compiled on/for many
systems, the following documentation explains how to do it on POSIX
ones. It is primarily thought for GNU/Linux systems and has been tested
on recent releases of
[Debian](https://debian.org)/[Ubuntu](https://ubuntu.com) and
[Fedora](https://getfedora.org).

Dependencies
------------

The Kiwix tools rely on a few third party software libraries. They are
prerequisites to the Kiwix tools compilation. Therefore, following
libraries need to be available:
* [libkiwix](https://github.com/kiwix/libkiwix) (package `libkiwix` on Debian/Ubuntu)
* [libzim](https://github.com/openzim/libzim) (package `libzim` on Debian/Ubuntu)

These dependencies may or may not be packaged by your operating
system. They may also be packaged but only in an older version. They
may be also packaged but without providing a static version. The
compilation script will tell you if one of them is missing or too old.
In the worse case, you will have to download and compile bleeding edge
version by hand.

If you want to install these dependencies locally, then use the
kiwix-tools directory as install prefix.

If you want to compile Kiwix tools statically, the dependencies should
be compiled statically (provide a `lib...a` library), for example by
using `--enable-static` with `./configure`.

Environment
-------------

The Kiwix tools build using [Meson](http://mesonbuild.com/) version
0.43 or higher. Meson relies itself on Ninja, pkg-config and a few other
compilation tools. Install them first:
* [Meson](http://mesonbuild.com/)
* [Ninja](https://ninja-build.org/)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)

These tools should be packaged if you use a cutting edge operating
system. If not, have a look to the [Troubleshooting](#Troubleshooting)
section.

Compilation
-----------

Once all dependencies are installed, you can compile Kiwix tools with:
```bash
meson . build
ninja -C build
```

By default, it will compile dynamic linked libraries. If you want
statically linked libraries, you can add `-Dstatic-linkage=true`
option to the Meson command.

Depending of you system, `ninja` may be called `ninja-build`.

Installation
------------

If you want to install the Kiwix tools, here we go:
```bash
ninja -C build install
```

You might need to run the command as `root` (or using `sudo`),
depending on where you want to install the Kiwix tools. After the
installation succeeded, you may need to run ldconfig (as `root`).

Uninstallation
------------

If you want to uninstall the Kiwix tools:
```bash
ninja -C build uninstall
```

Like for the installation, you might need to run the command as `root`
(or using `sudo`).

Docker
------

An official Docker image of the Kiwix tools can be found on 
[GHCR](https://ghcr.io/kiwix/kiwix-tools). A
`kiwix-serve` dedicated Docker image [exists
too](https://ghcr.io/kiwix/kiwix-serve).

Troubleshooting
---------------

If you need to install Meson "manually":
```bash
virtualenv -p python3 ./ # Create virtualenv
source bin/activate      # Activate the virtualenv
pip3 install meson       # Install Meson
hash -r                  # Refresh bash paths
```

If you need to install Ninja "manually":
```bash
git clone git://github.com/ninja-build/ninja.git
cd ninja
git checkout release
./configure.py --bootstrap
mkdir ../bin
cp ninja ../bin
cd ..
```

If the compilation still fails, you might need to get a more recent
version of a dependency than the one packaged by your Linux
distribution. Try then with a source tarball distributed by the
problematic upstream project or even directly from the source code
repository.

License
-------

[GPLv3](https://www.gnu.org/licenses/gpl-3.0) or later, see
[COPYING](COPYING) for more details.
