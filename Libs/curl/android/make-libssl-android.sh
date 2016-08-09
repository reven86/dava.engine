#! /bin/bash

# Internal build script for libssl and libcrypto, should be called from build-android.sh
#
# Required environment variables:
#   ANDROID_NDK_ROOT    - where Android NDK is located
#   LIBSSL_OUTPUT_DIR   - where to place compiled libssl.a and libcrypto.a files.
#                         Directory with arch name is created inside LIBCURL_OUTPUT_DIR
#                         (e.g. LIBCURL_OUTPUT_DIR/armeabi-v7a)
#   ARCH_LIST           - list of architectures to build
#   ANDROID_API         - android API level

if [ -z "$ARCH_LIST" ] ; then
    echo "error: ARCH_LIST is not specified"
    exit 1
fi
if [ -z "$ANDROID_NDK_ROOT" ] || [ ! -d "$ANDROID_NDK_ROOT" ] ; then
    echo "error: ANDROID_NDK_ROOT is not found or not set ($ANDROID_NDK_ROOT)"
    exit 1
fi
if [ -z "$ANDROID_API" ] ; then
    echo "error: ANDROID_API is not set"
    exit 1
fi

for ARCH in $ARCH_LIST ;
do
    echo "info: building for $ARCH"

    #####  Prepare android environment  ############################################
    if [ "$ARCH" == "armeabi-v7a" ] ; then
        ANDROID_ARCH="arch-arm"
        ANDROID_TOOLCHAIN_NAME="arm-linux-androideabi-4.9"
        CROSS_COMPILE="arm-linux-androideabi-"
        MACHINE="armv7"
    elif [ "$ARCH" == "x86" ] ; then
        ANDROID_ARCH="arch-x86"
        ANDROID_TOOLCHAIN_NAME="x86-4.9"
        CROSS_COMPILE="i686-linux-android-"
        MACHINE="x86"
    #elif [ "$ARCH" == "aarch64" ] ; then
    #    ANDROID_ARCH="arch-arm64"
    #    ANDROID_TOOLCHAIN_NAME="aarch64-linux-android-4.9"
    #    CROSS_COMPILE="aarch64-linux-android-"
    #    MACHINE="armv7"
    else
        echo "error: ARCH is not recognized ($ARCH)"
        exit 1
    fi

    for HOST in "darwin-x86_64" "darwin-x86" "linux-x86_64" "linux-x86"
    do
        if [ -d "$ANDROID_NDK_ROOT/toolchains/$ANDROID_TOOLCHAIN_NAME/prebuilt/$HOST/bin" ]; then
            ANDROID_TOOLCHAIN="$ANDROID_NDK_ROOT/toolchains/$ANDROID_TOOLCHAIN_NAME/prebuilt/$HOST/bin"
            break
        fi
    done
    SYSROOT="$ANDROID_NDK_ROOT/platforms/$ANDROID_API/$ANDROID_ARCH"

    if [ ! -d "$ANDROID_TOOLCHAIN" ] ; then
        echo "error: ANDROID_TOOLCHAIN dir not found ($ANDROID_TOOLCHAIN)"
        exit 1
    fi
    if [ ! -d "$SYSROOT" ] ; then
        echo "error: SYSROOT dir not found ($SYSROOT)"
        exit 1
    fi

    export PATH="$ANDROID_TOOLCHAIN":"$PATH"
    export SYSROOT="$SYSROOT"

    export CFLAGS="--sysroot=$SYSROOT"

    export CROSS_COMPILE="$CROSS_COMPILE"
    export ANDROID_DEV="$SYSROOT/usr"
    export MACHINE="$MACHINE"
    export SYSTEM="android"

    #####  Configure project  ######################################################
    make clean
    chmod a+x ./config

    perl -pi -e 's/install: all install_docs install_sw/install: install_docs install_sw/g' Makefile.org
    ./config no-shared \
             no-comp \
             no-hw \
             no-engine
    # no-ssl2 no-ssl3 ?????

    if [ $? -ne 0 ] ; then
        echo "error: configure failed"
        exit 1
    fi

    #####  Make project  ###########################################################
    make -j 8 depend && make -j 8 all
    if [ $? -ne 0 ] ; then
        echo "error: make failed"
        exit 1
    fi

    #####  Copy generated files to desired location  ###############################
    OUTPUT_FILE_LIST="libssl.a libcrypto.a"

    OUTPUT_DIR="$LIBSSL_OUTPUT_DIR/$ARCH"
    mkdir -p "$OUTPUT_DIR"
    if [ $? -ne 0 ] ; then
        echo "error: failed to create OUTPUT_DIR ($OUTPUT_DIR)"
        exit 1
    fi

    echo "info: copy $OUTPUT_FILE_LIST to $OUTPUT_DIR"
    for F in $OUTPUT_FILE_LIST ; do
        cp "$F" "$OUTPUT_DIR"
    done
done
