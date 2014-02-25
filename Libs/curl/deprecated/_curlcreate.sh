#!/bin/sh 

# Xcode->Pref->Download->Command Line Tools - Instale
# Unpack curl-7.25.0.tar & copy this script to unpack folder
# Terminal set folder to unpack folder & "bash _curlcreate.sh"

export SDK=5.0

buildit()
{
target=$1
        platform=$2

        export CC=/Developer/Platforms/${platform}.platform/Developer/usr/bin/gcc
        export CFLAGS="-arch ${target} -isysroot /Developer/Platforms/${platform}.platform/Developer/SDKs/${platform}${SDK}.sdk"
        export CPP="/Developer/Platforms/${platform}.platform/Developer/usr/bin/llvm-cpp-4.2"
        export AR=/Developer/Platforms/${platform}.platform/Developer/usr/bin/ar
        export RANLIB=/Developer/Platforms/${platform}.platform/Developer/usr/bin/ranlib

        ./configure --disable-shared --without-ssl --without-libssh2 --without-ca-bundle --without-ldap --disable-ldap \
                --host=${target}-apple-darwin10

        make clean
        make
        $AR rv libcurl.${target}.a lib/*.o
}

buildit1()
{
target=$1
platform=$2

export CC=/Developer/usr/bin/gcc
export CFLAGS="-arch ${target} -isysroot /Developer/SDKs/${platform}.sdk"
export CPP="/Developer/usr/bin/llvm-cpp-4.2"
export AR=/Developer/usr/bin/ar
export RANLIB=/Developer/usr/bin/ranlib

./configure --disable-shared --without-ssl --without-libssh2 --without-ca-bundle --without-ldap --disable-ldap \
--host=${target}-apple-darwin10

make clean
make
$AR rv libcurl.${target}.a lib/*.o
}


buildit armv6 iPhoneOS
buildit armv7 iPhoneOS
buildit i386 iPhoneSimulator
buildit1 x86_64 MacOSX10.7

lipo -create libcurl.armv7.a libcurl.armv6.a libcurl.i386.a libcurl.x86_64.a -output libcurl_iOS_mac.a
