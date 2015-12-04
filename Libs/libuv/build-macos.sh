#! /bin/bash

# remove ./out and xcode project
rm -rf ./out
rm -rf ./uv.xcodeproj

# generate xcode project
python ./gyp_uv.py -f xcode

xcodebuild -project uv.xcodeproj -configuration Release -target libuv \
    ARCHS="x86_64" \
    CONFIGURATION_BUILD_DIR="./out/macos_release" \
    CONFIGURATION_TEMP_DIR="./out/macos_release/conf_temp" \
    TARGET_TEMP_DIR="./out/macos_release/target_temp"

xcodebuild -project uv.xcodeproj -configuration Debug -target libuv \
    ARCHS="x86_64" \
    CONFIGURATION_BUILD_DIR="./out/macos_debug" \
    CONFIGURATION_TEMP_DIR="./out/macos_debug/conf_temp" \
    TARGET_TEMP_DIR="./out/macos_debug/target_temp"

mkdir -p ./bin/macos/release
cp ./out/macos_release/libuv.a ./bin/macos/release/libuv.a

mkdir -p ./bin/macos/debug
cp ./out/macos_debug/libuv.a ./bin/macos/debug/libuv.a
