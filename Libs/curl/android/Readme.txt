To build curl with crypto & ssl you need to follow these steps:
0. Set the NDK_ROOT environment variable
1. Copy curl and openssl sources in that one directory
2. Copy build-script directory to that directory
3. Run build-script/build_Android.sh

Tested on curl 7.34 & openssl 1.0.1t. You probably need to modify mk files when these libs will be updated
Original script was taken from https://github.com/gcesarmza/curl-android-ios