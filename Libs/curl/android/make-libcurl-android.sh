#! /bin/bash

# Internal build script for libcurl, should be called from build-android.sh
#
# Required environment variables:
#   ANDROID_NDK_ROOT    - where Android NDK is located
#   LIBCURL_OUTPUT_DIR  - where to place compiled libcurl.a file. Directory with arch name
#                         is created inside LIBCURL_OUTPUT_DIR (e.g. LIBCURL_OUTPUT_DIR/armeabi-v7a)
#   LIBSSL_INCLUDE_DIR  - where headers for libssl are located
#   LIBSSL_LIBRARY_DIR  - where libssl.a and libcrypto.a are located, should not include arch tail,
#                         e.g. if openssl compiled lib files are in /lib/openssl/armeabi-v7a then
#                         LIBSSL_LIBRARY_DIR should point to /lib/openssl
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

    if [ "$ARCH" == "armeabi-v7a" ] ; then
        ANDROID_ARCH="arch-arm"
        ANDROID_TOOLCHAIN_NAME="arm-linux-androideabi-4.9"
        CROSS_COMPILE="arm-linux-androideabi"
    elif [ "$ARCH" == "x86" ] ; then
        ANDROID_ARCH="arch-x86"
        ANDROID_TOOLCHAIN_NAME="x86-4.9"
        CROSS_COMPILE="i686-linux-android"
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

    export CPPFLAGS="--sysroot=$SYSROOT -I$LIBSSL_INCLUDE_DIR"
    export CFLAGS="--sysroot=$SYSROOT"
    export LDFLAGS="-L$LIBSSL_LIBRARY_DIR/$ARCH"
    export LIBS="-lssl -lcrypto"

    export AR="$CROSS_COMPILE-ar"
    export AS="$CROSS_COMPILE-as"
    export CPP="$CROSS_COMPILE-cpp"
    export CC="$CROSS_COMPILE-gcc"
    export CXX="$CROSS_COMPILE-c++"
    export LD="$CROSS_COMPILE-ld"
    export NM="$CROSS_COMPILE-nm"
    export RANLIB="$CROSS_COMPILE-ranlib"

    #####  Configure project  ######################################################
    make clean
    chmod a+x ./configure
    
    # configure can issue the following warnings which can be safely ignored
    # WARNING: the previous check could not be made default was used
    # WARNING: skipped the /dev/urandom detection when cross-compiling
    # WARNING: skipped the ca-cert path detection when cross-compiling
    # WARNING: using cross tools not prefixed with host triplet
    # WARNING: This libcurl built is probably not ABI compatible with previous
    # WARNING: builds! You MUST read lib/README.curl_off_t to figure it out.
    # SONAME bump: yes - WARNING: this library will be built with the SONAME
    #              number bumped due to (a detected) ABI breakage.
    #              See lib/README.curl_off_t for details on this.
    ./configure -q \
                --host="${CROSS_COMPILE}" \
                --target="${CROSS_COMPILE}" \
                --with-zlib \
                --with-ssl \
                --enable-static \
                --disable-shared \
                --disable-verbose \
                --enable-threaded-resolver \
                --enable-libgcc \
                --enable-ipv6 \
                --disable-gopher \
                --disable-file \
                --disable-imap \
                --disable-ldap \
                --disable-ldaps \
                --disable-pop3 \
                --disable-dict \
                --disable-rtsp \
                --disable-smtp \
                --disable-telnet \
                --disable-tftp \
                --without-gnutls \
                --without-libidn \
                --without-librtmp
    if [ $? -ne 0 ] ; then
        echo "error: configure failed"
        echo "LIBSSL_LIBRARY_DIR=$LIBSSL_LIBRARY_DIR"
        echo "LDFLAGS=$LDFLAGS"
        exit 1
    fi

    #####  Make project  ###########################################################
    make -j 8
    if [ $? -ne 0 ] ; then
        echo "error: make failed"
        exit 1
    fi

    #####  Copy generated files to desired location  ###############################
    OUTPUT_FILE_LIST="libcurl.a"

    OUTPUT_DIR="$LIBCURL_OUTPUT_DIR/$ARCH"
    mkdir -p "$OUTPUT_DIR"
    if [ $? -ne 0 ] ; then
        echo "error: failed to create OUTPUT_DIR ($OUTPUT_DIR)"
        exit 1
    fi

    echo "info: copy $OUTPUT_FILE_LIST to $OUTPUT_DIR"
    for F in $OUTPUT_FILE_LIST ; do
        cp "lib/.libs/$F" "$OUTPUT_DIR"
    done
done
