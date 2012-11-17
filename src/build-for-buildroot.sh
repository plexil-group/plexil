#! /bin/bash
# How to cross-compile Plexil with buildroot

# Substitute the appropriate triplet.
TARGET=arm-unknown-linux-uclibcgnueabi

# Substitute the appropriate paths.
PLEXIL_HOME=/home/cfry/src/plexil-3
BUILDROOT_HOME=/home/cfry/buildroot/buildroot-2012.08

# These should not need to be changed.
TOOLCHAIN_ROOT=${BUILDROOT_HOME}/output/host
TARGET_GCC=${TOOLCHAIN_ROOT}/usr/bin/${TARGET}-gcc
TARGET_GXX=${TOOLCHAIN_ROOT}/usr/bin/${TARGET}-g++
TARGET_NM=${TOOLCHAIN_ROOT}/usr/bin/${TARGET}-nm
BUILD_ROOT=${TOOLCHAIN_ROOT}/output/build/plexil-3.0.0a1
TARGET_ROOT=${BUILDROOT_HOME}/output/target
TARGET_INCLUDES=${TARGET_ROOT}/usr/include

export PATH=${PLEXIL_HOME}/src:${TOOLCHAIN_ROOT}/usr/bin:${TOOLCHAIN_ROOT}/usr/sbin:${PATH}

mkdir -p $BUILD_ROOT
cd $BUILD_ROOT
${PLEXIL_HOME}/src/configure --target $TARGET --host=$TARGET \
 CC=$TARGET_GCC \
 CXX=$TARGET_GXX \
 NM=$TARGET_NM \
 --prefix=${TARGET_ROOT}/usr
 --enable-module-tests --enable-test-exec
make
make install

