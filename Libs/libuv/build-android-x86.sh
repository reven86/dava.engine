#! /bin/bash

# clone standalone toolchain
export TOOLCHAIN=$PWD/android-toolchain-x86
mkdir -p $TOOLCHAIN

$ANDROID_NDK_ROOT/build/tools/make-standalone-toolchain.sh \
    --toolchain=x86-clang3.6 \
    --arch=x86 \
    --install-dir=$TOOLCHAIN \
    --platform=android-14

export PATH=$TOOLCHAIN/bin:$PATH
export AR=i686-linux-android-ar
export CC=clang
export CXX=clang++
export LINK=i686-linux-android-g++
export PLATFORM=android

# remove ./out
rm -rf ./out

# generate makefile in ./out
./gyp_uv.py -Dtarget_arch=x86 -DOS=android -f make-android

# build
make -C out libuv BUILDTYPE=Debug
make -C out libuv BUILDTYPE=Release

# copy libuv.a to known location ./bin/android
mkdir -p ./bin/android/x86/debug
cp ./out/Debug/libuv.a ./bin/android/x86/debug/libuv.a

mkdir -p ./bin/android/x86/release
cp ./out/Release/libuv.a ./bin/android/x86/release/libuv.a
