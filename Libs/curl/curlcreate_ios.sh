#!/bin/bash

# Install Command Line Tools for Xcode. Xcode -> Preferences -> Download -> Command Line Tools - Install
# Unpack curl-7.28.1.tar.gz & copy this script to unpacked folder
# Set appropriate SDK version at line 10: export SDK=X.Y
# In terminal change dir to unpacked folder & "bash curlcreate_ios.sh"

set -e

export SDK=7.0

checkExists() {

    if [ ! -e $1 ]
    then
        echo "Didn't find: $1 -- try to locate parts of this to see how to fix the path"
        exit 1
    else 
        echo "using $1"
    fi

}

buildit() {

    target=$1
    platform=$2

    root="/Applications/Xcode.app/Contents/Developer/Platforms/${platform}.platform/Developer"
    oldRoot="/Developer/Platforms/${platform}.platform/Developer"

    if [ ! -d "${root}" ]
    then
        root="${oldRoot}"
    fi

    if [ ! -d "${root}" ]
    then
        echo " "
        echo "Oopsie.  You don't have an iOS SDK root in either of these locations: "
        echo "   ${root} "
        echo "   ${oldRoot}"
        echo " "
        echo "If you have 'locate' enabled, you might find where you have it installed with:"
        echo "   locate iPhoneOS.platform | grep -v 'iPhoneOS.platform/'"
        echo " "
        echo "and alter the 'root' variable in the script -- or install XCode if you can't find it... "
        echo " "
        exit 1
    fi

    export CC="/usr/bin/clang"
    export CFLAGS="-arch ${target} -isysroot ${root}/SDKs/${platform}${SDK}.sdk -miphoneos-version-min=6.0"
    export CXX="/usr/bin/clang"
    export AR="/usr/bin/ar"
    export RANLIB="/usr/bin/ranlib"

    checkExists ${CC}
    checkExists ${root}/SDKs/${platform}${SDK}.sdk
    checkExists ${CPP}
    checkExists ${AR}
    checkExists ${RANLIB}

    ./configure --disable-shared --enable-static --with-darwinssl --without-libssh2 --without-ca-bundle --without-ldap --disable-ldap --host=${target}-apple-darwin10 --enable-threaded-resolver --enable-ipv6

    make clean
    make
    $AR rv libcurl.${target}.a lib/*.o
}

buildit armv7 iPhoneOS
buildit armv7s iPhoneOS
buildit i386 iPhoneSimulator

lipo -create libcurl.armv7.a libcurl.armv7s.a libcurl.i386.a -output libcurl_ios.a

rm -f libcurl.armv7.a libcurl.armv7s.a libcurl.i386.a