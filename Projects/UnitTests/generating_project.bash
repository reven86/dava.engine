#cmake -DCMAKE_TOOLCHAIN_FILE=iOS-toolchain.cmake -G "Xcode" ..


BUILD_IOS="_build_ios" 
BUILD_MAC="_build_mac" 

HOME_DIR=$(pwd)

cd $HOME_DIR
cmake -E make_directory $BUILD_IOS
cd $BUILD_IOS
cmake  -G"Xcode" -DCMAKE_TOOLCHAIN_FILE="$HOME_DIR\..\..\Sources\CMake\Toolchains\ios.toolchain.cmake" ..

cd $HOME_DIR
cmake -E make_directory $BUILD_MAC
cd $BUILD_MAC
cmake  -G"Xcode" ..

