mkdir _tmp_build
cd _tmp_build
cmake -G "Visual Studio 10" ..
cmake --build . --target install

::cmake cmake -G"Eclipse CDT4 - MinGW Makefiles" -DANDROID_NATIVE_API_LEVEL=android-9 -DCMAKE_TOOLCHAIN_FILE="D:\work\dava.framework_NEW\Source\CMake\Toolchains\android.toolchain.cmake" ..
::cmake --build . --target install