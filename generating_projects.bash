#cmake -DCMAKE_TOOLCHAIN_FILE=iOS-toolchain.cmake -G "Xcode" ..


PATH_BUILD="_build/dava.framework_ios" 
HOME_DIR=$(pwd)


cmake -E make_directory $PATH_BUILD
cd $PATH_BUILD
cmake  -G"Xcode" -DCMAKE_TOOLCHAIN_FILE="$HOME_DIR\Sources\CMake\Toolchains\ios.toolchain.cmake" ../../Sources




#cmake -E make_directory $PATH_BUILD
#cmake -E chdir $PATH_BUILD cmake  -G"Xcode" -DCMAKE_TOOLCHAIN_FILE="$HOME_DIR\Sources\CMake\Toolchains\ios.toolchain.cmake" ..\..\Sources


