#!/usr/bin/env bash

set -e

BUILD_DIR=${HOME}/BUILD_${PLATFORM}
INSTALL_DIR=${BUILD_DIR}/INSTALL


case ${PLATFORM} in
    "native_static")
         MESON_OPTION="--default-library=static"
         ;;
    "native_dyn")
         MESON_OPTION="--default-library=shared"
         ;;
    "win32_static")
         MESON_OPTION="--default-library=static --cross-file ${BUILD_DIR}/meson_cross_file.txt"
         ;;
    "win32_dyn")
         MESON_OPTION="--default-library=shared --cross-file ${BUILD_DIR}/meson_cross_file.txt"
         ;;
    "android_arm")
         MESON_OPTION="-Dandroid=true --default-library=shared --cross-file ${BUILD_DIR}/meson_cross_file.txt"
         ;;
    "android_arm64")
         MESON_OPTION="-Dandroid=true --default-library=shared --cross-file ${BUILD_DIR}/meson_cross_file.txt"
         ;;

esac

cd ${TRAVIS_BUILD_DIR}
export PKG_CONFIG_PATH=${INSTALL_DIR}/lib/x86_64-linux-gnu/pkgconfig
export PATH=${INSTALL_DIR}/bin:$PATH
meson . build -Dctpp2-install-prefix=${INSTALL_DIR} ${MESON_OPTION}
cd build
ninja
