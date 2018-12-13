Kiwix tools
===========

The Kiwix tools is a collection of Kiwix related command line tools.

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Kiwix tools compilation itself, we recommend to have a look
to [kiwix-build](https://github.com/kiwix/kiwix-build).

Preamble
--------

Although the Kiwix tools can be compiled/cross-compiled on/for many
sytems, the following documentation explains how to do it on POSIX
ones. It is primarly thought for GNU/Linux systems and has been tested
on recent releases of Ubuntu and Fedora.

Dependencies
------------

The Kiwix tools rely on a few third party software libraries. They are
prerequisites to the Kiwix tools compilation. Therefore, following
libraries need to be available:

* Kiwix lib ....................... https://github.com/kiwix/kiwix-lib
(no package so far)
* Libmicrohttpd .......... https://www.gnu.org/software/libmicrohttpd/
(package libmicrohttpd-dev on Ubuntu)
* CTPP2 ..................................... http://ctpp.havoc.ru/en/
(package libctpp2-dev on Ubuntu)
* Zlib .......................................... http://www.zlib.net/
(package zlib1g-dev on Ubuntu)

These dependencies may or may not be packaged by your operating
system. They may also be packaged but only in an older version. They
may be also packaged but without providing a static version. The
compilation script will tell you if one of them is missing or too old.
In the worse case, you will have to download and compile bleeding edge
version by hand.

If you want to install these dependencies locally, then use the
kiwix-tools directory as install prefix.

If you want to compile Kiwix tools statically, the dependencies should
be compile statically (provide a lib...a library), for example by
using "--enable-static" with "./configure".

If you compile manually Libmicrohttpd, you might need to compile it
without GNU TLS, a bug here will empeach further compilation of Kiwix
tools otherwise.

Environment
-------------

The Kiwix tools build using [Meson](http://mesonbuild.com/) version
0.39 or higher. Meson relies itself on Ninja, pkg-config and few other
compilation tools. Install them first:
* Meson
* Ninja
* Pkg-config

These tools should be packaged if you use a cutting edge operating
system. If not, have a look to the "Troubleshooting" section.

Compilation
-----------

Once all dependencies are installed, you can compile Kiwix tools with:
```
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

```
ninja -C build install
```

You might need to run the command as root (or using 'sudo'), depending
where you want to install the Kiwix tools. After the installation
succeeded, you may need to run ldconfig (as root).

Uninstallation
------------

If you want to uninstall the Kiwix tools:

```
ninja -C build uninstall
```

Like for the installation, you might need to run the command as root
(or using 'sudo').

Troubleshooting
---------------

If you need to install Meson "manually":
```
virtualenv -p python3 ./ # Create virtualenv
source bin/activate      # Activate the virtualenv
pip3 install meson       # Install Meson
hash -r                  # Refresh bash paths
```

If you need to install Ninja "manually":
```
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

GPLv3 or later, see COPYING for more details.
