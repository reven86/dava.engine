#!/bin/bash

rm -R android-toolchain-gcc4.8
$NDK_ROOT/build/tools/make-standalone-toolchain.sh --platform=android-14 --install-dir=android-toolchain-gcc4.8 --toolchain=arm-linux-androideabi-4.8 --system=darwin-x86_64

# build openssl
pushd openssl
	export PATH=`pwd`/../android-toolchain-gcc4.8/bin:$PATH
	export CC=arm-linux-androideabi-gcc
	export CXX=arm-linux-androideabi-g++
	export AR=arm-linux-androideabi-ar
	export RANLIB=arm-linux-androideabi-ranlib
	./Configure android-armv7 no-shared || exit 1
	make clean && make build_crypto build_ssl -j $CPU_COUNT || exit 1
popd

# build curl
CURL='curl-7.34.0'
pushd curl
unzip $CURL.zip
pushd $CURL
	OPENSSL=`pwd`/../../openssl
	export CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
	export CPPFLAGS=-DANDROID
	export LDFLAGS="-march=armv7-a -Wl,--fix-cortex-a8 -L$OPENSSL"
	./configure --disable-shared --enable-static --host=arm-linux-androideabi --with-random=/dev/urandom --with-ssl=$OPENSSL --without-ca-bundle --without-ca-path --with-zlib --enable-threaded-resolver --enable-ipv6 $CURL_EXTRA || exit 1
	make clean && make -j $CPU_COUNT || exit 1
popd
popd

# copy headers
#cp -RL openssl/include/ include/
#mkdir include/curl
#cp curl/$CURL/include/curl/*.h include/curl

# copy library
cp -f openssl/libcrypto.a libs/android/armeabi-v7a/libcrypto_android.a 
cp -f openssl/libssl.a libs/android/armeabi-v7a/libssl_android.a
cp curl/curl-7.34.0/lib/.libs/libcurl.a libs/android/armeabi-v7a/libcurl_android.a

# clean
rm -R curl/$CURL
pushd openssl
	make clean
popd
rm -R android-toolchain-gcc4.8

$NDK_ROOT/build/tools/make-standalone-toolchain.sh --platform=android-14 --install-dir=android-toolchain-gcc4.8 --toolchain=x86-4.8 --system=darwin-x86_64

# build openssl
pushd openssl
	export PATH=`pwd`/../android-toolchain-gcc4.8/bin:$PATH
	export CC=i686-linux-android-gcc
	export CXX=i686-linux-android-g++
	export AR=i686-linux-android-ar
	export RANLIB=i686-linux-android-ranlib
	./Configure android-x86 || exit 1
	make clean && make build_crypto build_ssl -j $CPU_COUNT || exit 1
popd

# build curl
CURL='curl-7.34.0'
pushd curl
unzip $CURL.zip
pushd $CURL
	OPENSSL=`pwd`/../../openssl
	export CFLAGS="-march=i686"
	export CPPFLAGS=-DANDROID
	export LDFLAGS="-march=i686 -L$OPENSSL"
	./configure --disable-shared --enable-static --host=i686-linux-android --with-random=/dev/urandom --with-ssl=$OPENSSL --without-ca-bundle --without-ca-path --with-zlib --enable-threaded-resolver --enable-ipv6 $CURL_EXTRA || exit 1
	make clean && make -j $CPU_COUNT || exit 1
popd
popd

# copy headers
#cp -RL openssl/include/ include/
#mkdir include/curl
#cp curl/$CURL/include/curl/*.h include/curl

# copy library
cp -f openssl/libcrypto.a libs/android/x86/libcrypto_android.a 
cp -f openssl/libssl.a libs/android/x86/libssl_android.a
cp curl/curl-7.34.0/lib/.libs/libcurl.a libs/android/x86/libcurl_android.a

# clean
rm -R curl/$CURL
pushd openssl
	make clean
popd
rm -R android-toolchain-gcc4.8