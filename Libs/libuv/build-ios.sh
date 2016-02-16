#! /bin/bash

# remove ./out and xcode project
rm -rf ./out
rm -rf ./uv.xcodeproj

# generate xcode project
python ./gyp_uv.py -f xcode

# release build for ios
xcodebuild  -project uv.xcodeproj -configuration Release -target libuv \
    ARCHS="arm64 armv7 armv7s" \
    VALID_ARCHS="arm64 armv7 armv7s" \
    SUPPORTED_PLATFORMS="iphoneos" \
    SDKROOT="iphoneos" \
    TARGETED_DEVICE_FAMILY="1,2" \
    IPHONEOS_DEPLOYMENT_TARGET="7.0" \
    CONFIGURATION_BUILD_DIR="./out/ios_release" \
    CONFIGURATION_TEMP_DIR="./out/ios_release/conf_temp" \
    TARGET_TEMP_DIR="./out/ios_release/target_temp"

# release build for simulator
xcodebuild  -project uv.xcodeproj -configuration Release -target libuv \
    ARCHS="i386 x86_64" \
    VALID_ARCHS="i386 x86_64" \
    SUPPORTED_PLATFORMS="iphonesimulator" \
    SDKROOT="iphonesimulator" \
    TARGETED_DEVICE_FAMILY="1,2" \
    IPHONEOS_DEPLOYMENT_TARGET="7.0" \
    CONFIGURATION_BUILD_DIR="./out/iossimu_release" \
    CONFIGURATION_TEMP_DIR="./out/iossimu_release/conf_temp" \
    TARGET_TEMP_DIR="./out/iossimu_release/target_temp"

# combine ios and simulator libs
mkdir -p ./bin/ios/release
lipo ./out/ios_release/libuv.a ./out/iossimu_release/libuv.a -create -output ./bin/ios/release/libuv.a


# debug build for ios
xcodebuild  -project uv.xcodeproj -configuration Debug -target libuv \
    ARCHS="arm64 armv7 armv7s" \
    VALID_ARCHS="arm64 armv7 armv7s" \
    SUPPORTED_PLATFORMS="iphoneos" \
    SDKROOT="iphoneos" \
    TARGETED_DEVICE_FAMILY="1,2" \
    IPHONEOS_DEPLOYMENT_TARGET="7.0" \
    CONFIGURATION_BUILD_DIR="./out/ios_debug" \
    CONFIGURATION_TEMP_DIR="./out/ios_debug/conf_temp" \
    TARGET_TEMP_DIR="./out/ios_debug/target_temp"

# debug build for simulator
xcodebuild  -project uv.xcodeproj -configuration Debug -target libuv \
    ARCHS="i386 x86_64" \
    VALID_ARCHS="i386 x86_64" \
    SUPPORTED_PLATFORMS="iphonesimulator" \
    SDKROOT="iphonesimulator" \
    TARGETED_DEVICE_FAMILY="1,2" \
    IPHONEOS_DEPLOYMENT_TARGET="7.0" \
    CONFIGURATION_BUILD_DIR="./out/iossimu_debug" \
    CONFIGURATION_TEMP_DIR="./out/iossimu_debug/conf_temp" \
    TARGET_TEMP_DIR="./out/iossimu_debug/target_temp"

# combine ios and simulator libs
mkdir -p ./bin/ios/debug
lipo ./out/ios_debug/libuv.a ./out/iossimu_debug/libuv.a -create -output ./bin/ios/debug/libuv.a
