#! /bin/bash

# Script to build libssl and/or libcurl for android
#
# Set up variables before running script:
#     BUILD_LIBSSL        - whether to build libssl (to exclude from build set to 0)
#     BUILD_LIBCURL       - whether to build libcurl (to exclude from build set to 0)
#     ARCH_LIST           - list of desired architectures (default is "armeabi-v7a x86")
#     ANDROID_API         - android API level (default is android-14)
#   libssl specific:
#     LIBSSL_DIR          - where libssl source files are located, necessary if BUILD_LIBSSL is 1
#     LIBSSL_OUTPUT_DIR   - where to place libssl *.a files
#   libcurl specific:
#     LIBCURL_DIR         - where libcurl source files are located, necessary if BUILD_LIBCURL is 1
#     LIBCURL_OUTPUT_DIR  - where to place libcurl *.a files
#     LIBSSL_INCLUDE_DIR  - where libssl header are located, default is LIBSSL_DIR/include.
#                           Usually should be set if building only libcurl with already prebuilt libssl
#     LIBSSL_LIBRARY_DIR  - where libssl lib files are located, default is LIBSSL_OUTPUT_DIR
#                           Usually should be set if building only libcurl with already prebuilt libssl

BUILD_LIBSSL=1
BUILD_LIBCURL=1

export LIBSSL_DIR="openssl-1.0.1t"
export LIBCURL_DIR="curl-7.34.0"

export ARCH_LIST="armeabi-v7a x86"
export ANDROID_API="android-14"

export LIBSSL_OUTPUT_DIR="$PWD/_build"
export LIBCURL_OUTPUT_DIR="$PWD/_build"

export LIBSSL_INCLUDE_DIR="$PWD/$LIBSSL_DIR/include"
export LIBSSL_LIBRARY_DIR="$LIBSSL_OUTPUT_DIR"

if [ -n "$BUILD_LIBSSL" ] && [ "$BUILD_LIBSSL" != "0" ] ; then
    echo "info: building libssl"
    cd "$LIBSSL_DIR"
    ../make-libssl-android.sh
    if [ $? -ne 0 ] ; then
        echo "error: failed to build libssl"
        exit 1
    fi
    cd ..
fi

if [ -n "$BUILD_LIBCURL" ] && [ "$BUILD_LIBCURL" != "0" ] ; then
    echo "info: building libcurl"
    cd "$LIBCURL_DIR"
    ../make-libcurl-android.sh
    if [ $? -ne 0 ] ; then
        echo "error: failed to build libcurl"
        exit 1
    fi
    cd ..
fi
