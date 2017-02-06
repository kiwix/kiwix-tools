Kiwix tools
===========

The Kiwix tools gathers kiwix command line tools.

Disclaimer
----------

This document assumes you have a little knowledge about software
compilation. If you experience difficulties with the dependencies or
with the Kiwix libary compilation itself, we recommend to have a look
to [kiwix-build](https://github.com/kiwix/kiwix-build).

Preamble
--------

Although the Kiwix tools can be compiled/cross-compiled on/for many
sytems, the following documentation explains how to do it on POSIX
ones. It is primarly though for GNU/Linux systems and has been tested
on recent releases of Ubuntu and Fedora.

Dependencies
------------

The Kiwix tools rely on a few third parts software libraries. They
are prerequisites to the Kiwix library compilation. Following
libraries need to be available:

* Kiwix lib ....................... https://github.com/kiwix/kiwix-lib
(no package for now)
* Libmicrohttpd .......... https://www.gnu.org/software/libmicrohttpd/
(package libmicrohttpd-dev on Ubuntu)
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

Environnement
-------------

The Kiwix library builds using [Meson](http://mesonbuild.com/) version
0.34 or higher. Meson relies itself on Ninja, pkg-config and few other
compilation tools.

Install first the few common compilation tools:
* Automake
* Libtool
* Virtualenv
* Pkg-config

Then install Meson itself:
```
virtualenv -p python3 ./ # Create virtualenv
source bin/activate      # Activate the virtualenv
pip install meson        # Install Meson
hash -r                  # Refresh bash paths
```

Finally download and build Ninja locally:
```
git clone git://github.com/ninja-build/ninja.git
cd ninja
git checkout release
./configure.py --bootstrap
mkdir ../bin                      
cp ninja ../bin
cd ..
```

Compilation
-----------

Once all dependencies are installed, you can compile kiwix-lib with:
```
mkdir build
meson . build
cd build
ninja
```

By default, it will compile dynamic linked libraries. If you want
statically linked libraries, you can add `-Dstatic-linkage=true`
option to the Meson command.

Depending of you system, `ninja` may be called `ninja-build`.

Installation
------------

If you want to install the Kiwix tools you just have compiled on your
system, here we go:

```
ninja install
cd ..
```

You might need to run the command as root, depending where you want to
install the libraries.

License
-------

GPLv3 or later, see COPYING for more details.
