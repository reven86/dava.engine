@echo off
set START_DIR=%CD%
set SCRIPT_DIR=%~dp0
set SOURCE_DIR=%1/
set CMAKE_DIR=%~dp0cmake_build\bin\Debug\
set CURRENT_PLATFORM= %2

echo START_DIR=%START_DIR%
echo SCRIPT_DIR=%SCRIPT_DIR%
echo SOURCE_DIR=%SOURCE_DIR%
echo CMAKE_DIR=%CMAKE_DIR%
echo CURRENT_PLATFORM=%CURRENT_PLATFORM%

if not exist %CMAKE_DIR%\cmake.exe ( (cd %~dp0) & (call rebuild_cmake.bat) & (cd %START_DIR%) )

if not exist %CMAKE_DIR%\cmake.exe (echo "Can't find cmake.exe in %CMAKE_DIR%") & (exit)

if "%1" == "" (echo "Add path which contains CMakeLists.txt") & (exit)

if not exist "%SOURCE_DIR%CMakeLists.txt" (echo "Can't find CMakeLists.txt in %SOURCE_DIR%") & (exit)

if not "%2" == "ARM" (
if not "%2" == "Win32" (
if not "%2" == "Win64" (
if not "%2" == "" ( (echo "Add platform Win64 or ARM default Win32") & (exit) ) ) ) )

if "%2" == "" set CURRENT_PLATFORM=

if "%2" == "Win32" set CURRENT_PLATFORM=

%CMAKE_DIR%\cmake.exe -G "Visual Studio 14 2015%CURRENT_PLATFORM%" -DWINDOWS_UAP=true %SOURCE_DIR%

::%CMAKE_DIR%\cmake.exe -G "Visual Studio 14 2015" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 -DCMAKE_VS_WINRT_COMPONENT=FALSE -A"Win32|ARM|x64" -DCMAKE_VS_EFFECTIVE_PLATFORMS=Win32;ARM;x64 %SOURCE_DIR%
