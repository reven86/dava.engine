::
@echo off
call update_cmake.bat
set WORK_DIR_REPO=cmake_repo
if not exist %WORK_DIR_REPO% (echo "Can't find %WORK_DIR_REPO%.") & (exit)
::
set WORK_DIR_BUILD=cmake_build
if exist %WORK_DIR_BUILD% rmdir /s /q %WORK_DIR_BUILD%
mkdir %WORK_DIR_BUILD%
cd %WORK_DIR_BUILD%
cmake ../%WORK_DIR_REPO%
cmake --build .
