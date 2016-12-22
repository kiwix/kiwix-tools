Kiwix is an offline reader for Web content. It's especially thought to
make Wikipedia available offline.  This is done by reading the content
of the project stored in a file format ZIM, a high compressed open
format with additional meta-data.

This repository contains a set of tools to help development or usage
of kiwix.

# COMPILATION INSTRUCTIONS

Most of the compilation steps are handled by the kiwix-builder.py
script.

This script has been tested of Fedora 23 and Ubuntu 16.10

Take care, the paragraphs are about the *target platforms*. If you
want to build Kiwix for Android on a GNU/Linux system, you should
follow the instructions of the "Android" paragraph.

## GNU/Linux
Install pre-requisties in your distro, eg, in Debian based:

    sudo apt install git cmake meson python3-virtualenv virtualenvwrapper zlib1g-dev libicu-dev aria2 libtool

### Dynamic

Pretty simple once all dependencies are installed :

The archives and sources will be copied in your current working dir so
I recommend you to create a working directory:

    $ mkdir <MY_WORKING_DIR>
    $ cd <MY_WOKRING_DIR>

Once ready, just run the script with the install dir as argument:

    $ kiwix-builder.py <INSTALL_DIR>

The script will download and install all the needed dependencies and
kiwix related repository.

At the end of the script you will found the binaries in <INSTALL_DIR>/bin.

As it is a dynamic linked binary, you will need to add the lib directory to
LD_LIBRARY_PATH :

    $ LD_LIBRARY_PATH=<INSTALL_DIR>/lib <INSTALL_DIR>/bin/kiwix-serve

### Static

To build a static binary simply add the --build_static option to the 
kiwix-build.py script :

    $ kiwix-builder.py --build_static <INSTALL_DIR>

Notes: You cannot use the same working directory to build a dynamic and static.

Notes: At the end of the script, meson may raise a error when it install the
kiwix-server binary. This is a meson bug and it has been fixed here
(https://github.com/mesonbuild/meson/pull/1240) but your meson version may
not include this fix.
However, the kiwix-server binary is valid and ready to use.

## Mac OSX Universal

The script has not been tested on Mac OSX.

Please, if you have a Mac OSX, try to run the kiwix-builder script and
report errors or, better, fixes :)

## Android and Windows

Cross compilation and strange stuff are to come.

If you wich to help, you're welcome.

# CONTACT

Email: kiwix-developer@lists.sourceforge.net or contact@kiwix.org

IRC: #kiwix on irc.freenode.net  
(I'm hidding myself under the starmad pseudo)

You can use IRC web interface on http://chat.kiwix.org/

More... http://www.kiwix.org/wiki/Communication

# LEGAL & DISCLAIMER

Read 'COPYING' file
