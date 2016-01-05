@echo off
set WORK_DIR_REPO=cmake_repo
if exist %WORK_DIR_REPO% (
cd %WORK_DIR_REPO%
git pull https://github.com/ArtLapitski/CMake vs_extension_sdk_refs
git status
cd ..
) else (
git clone https://github.com/ArtLapitski/CMake %WORK_DIR_REPO%
cd %WORK_DIR_REPO%
git checkout vs_extension_sdk_refs
git status
cd ..
)