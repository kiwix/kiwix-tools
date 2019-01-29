#!/usr/bin/env bash

set -e

REPO_NAME=${TRAVIS_REPO_SLUG#*/}
ARCHIVE_NAME=deps_${TRAVIS_OS_NAME}_${PLATFORM}_${REPO_NAME}.tar.gz

# Packages.
case ${PLATFORM} in
     "native_static")
         PACKAGES="gcc python3.5 cmake libbz2-dev ccache zlib1g-dev uuid-dev"
         ;;
     "native_dyn")
         PACKAGES="gcc python3.5 cmake libbz2-dev ccache zlib1g-dev uuid-dev libmicrohttpd-dev"
         ;;
     "win32_static")
         PACKAGES="g++-mingw-w64-i686 gcc-mingw-w64-i686 gcc-mingw-w64-base mingw-w64-tools ccache python3.5"
         ;;
     "win32_dyn")
         PACKAGES="g++-mingw-w64-i686 gcc-mingw-w64-i686 gcc-mingw-w64-base mingw-w64-tools ccache python3.5"
         ;;
     "android_arm")
         PACKAGES="gcc python3.5 cmake ccache"
         ;;
     "android_arm64")
         PACKAGES="gcc python3.5 cmake ccache"
         ;;
esac

sudo apt-get update -qq
sudo apt-get install -qq python3-pip ${PACKAGES}
wget https://bootstrap.pypa.io/get-pip.py
python3.5 get-pip.py --user
python3.5 -m pip install --user --upgrade pip
python3.5 -m pip install --user meson

# Ninja
cd $HOME
git clone git://github.com/ninja-build/ninja.git
cd ninja
git checkout release
./configure.py --bootstrap
sudo cp ninja /bin

# Dependencies comming from kiwix-build.
cd ${HOME}
wget http://tmp.kiwix.org/ci/${ARCHIVE_NAME}
tar xf ${HOME}/${ARCHIVE_NAME}
