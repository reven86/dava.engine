This is libuv fork with Windows Universal Platform support from https://github.com/kkdaemon/libuv
git clone -b winuap_support https://github.com/kkdaemon/libuv.git

libuv original repository is at https://github.com/libuv/libuv

===============================================================
How to build

Unpack libuv.zip to some dir, e.g. libuv
Copy next files to libuv dir:
    build-ios.sh
    build-macos.sh

Install gyp into build/gyp, see libuv/README.md
    
Android
    enter to Android folder
    run ndk-build

iOS
    run build-ios.sh for arm64, armv7, armv7s, i386, x86_64 arch
    compiled library fat files are placed in libuv/bin/ios

Mac OS X
    run build-macos.sh
    compiled library files are placed in libuv/bin/macos

Win32
    run libuv\vcbuild.bat
    open and build solution libuv\uv.sln

Universal Windows Platform (Windows 10)
    open and build solution libuv/winuap/libuv.sln
    compiled library files are placed in libuv/winuap/lib
