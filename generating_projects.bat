@echo off
pushd %~dp0

set "dava_path=_build\dava.framework_win32"
set "dava_path_android=_build\dava.framework_android"
set "arch="
set "version=10"
set mydir=%cd%
echo on

::cmake -E make_directory %dava_path%
::cmake -E chdir %dava_path% cmake -G "Visual Studio %version%%arch%" %* ..\..\Sources\Engine
::cmake -E chdir %dava_path% cmake --build . 
::--target install

cmake -E make_directory %dava_path_android%
cmake -E chdir %dava_path_android% cmake %OPT% -G"Eclipse CDT4 - MinGW Makefiles" -DCMAKE_MAKE_PROGRAM="%ANDROID_NDK%\prebuilt\windows\bin\make.exe" -DANDROID_NATIVE_API_LEVEL=android-14 -DCMAKE_TOOLCHAIN_FILE="%mydir%\Sources\CMake\Toolchains\android.toolchain.cmake" ..\..\Sources\Engine
::cmake -E chdir %dava_path_android% cmake --build . 
::--target install

@popd
