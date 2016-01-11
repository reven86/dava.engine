#! /bin/bash

# clone standalone toolchain
export TOOLCHAIN=$PWD/android-toolchain-arm
mkdir -p $TOOLCHAIN

$ANDROID_NDK_ROOT/build/tools/make-standalone-toolchain.sh \
    --toolchain=arm-linux-androideabi-clang3.6 \
    --arch=arm \
    --install-dir=$TOOLCHAIN \
    --platform=android-14

export PATH=$TOOLCHAIN/bin:$PATH
export AR=arm-linux-androideabi-ar
export CC=clang
export CXX=clang++
export LINK=arm-linux-androideabi-g++
export PLATFORM=android

# remove ./out
rm -rf ./out

# generate makefile in ./out
./gyp_uv.py -Dtarget_arch=arm -DOS=android -f make-android

# build
make -C out libuv BUILDTYPE=Debug
make -C out libuv BUILDTYPE=Release

# copy libuv.a to known location ./bin/android
mkdir -p ./bin/android/arm/debug
cp ./out/Debug/libuv.a ./bin/android/arm/debug/libuv.a

mkdir -p ./bin/android/arm/release
cp ./out/Release/libuv.a ./bin/android/arm/release/libuv.a
