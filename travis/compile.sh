#!/usr/bin/env bash

set -e

BUILD_DIR=${HOME}/BUILD_${PLATFORM}
INSTALL_DIR=${BUILD_DIR}/INSTALL


case ${PLATFORM} in
    "native_static")
         MESON_OPTION="-Dstatic-linkage=true"
         ;;
    "native_dyn")
         MESON_OPTION=""
         ;;
    "win32_static")
         MESON_OPTION="-Dstatic-linkage=true --cross-file ${BUILD_DIR}/meson_cross_file.txt"
         ;;
    "win32_dyn")
         MESON_OPTION="--cross-file ${BUILD_DIR}/meson_cross_file.txt"
         ;;
esac

cd ${TRAVIS_BUILD_DIR}
export PKG_CONFIG_PATH=${INSTALL_DIR}/lib/x86_64-linux-gnu/pkgconfig
export PATH=${INSTALL_DIR}/bin:$PATH
meson . build ${MESON_OPTION}
cd build
ninja
