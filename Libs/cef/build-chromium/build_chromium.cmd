@echo off

:: Script to automatically download and build chromium embedded framework for x86 and x64
:: Script applies patch for chromium to run console processes without creating console window (for Windows)
:: After building is done script copies resultng binaries to dava.engine directories
::
:: Script is intented to run from DAVA_ENGINE_HOME\Libs\cef\build-chromium where DAVA_ENGINE_HOME is
:: path to dava.engine root. If script is launched from other location set variable DAVA_ENGINE_HOME to
:: proper value

:: python and git must be in PATH
:: File automate-git.py should be placed near script

:: Useful links to build chromium
:: https://bitbucket.org/chromiumembedded/cef/wiki/BranchesAndBuilding
:: https://bitbucket.org/chromiumembedded/cef/wiki/AutomatedBuildSetup.md
:: https://bitbucket.org/chromiumembedded/cef/wiki/MasterBuildQuickStart.md

::================================================================

:: Chromium branch to build, to build latest chomium specify trunk
set CHROMIUM_BRANCH=2526
:: Directory to download chromium sources
set CHROMUIM_DOWNLOAD_DIR=f:\chromium\chromium_git
set DAVA_ENGINE_HOME=..\..\..\..\dava.framework

:: Check where automate-git.py exists
if not exist automate-git.py (
    echo File 'automate-git.py' not found
    echo Download it from https://bitbucket.org/chromiumembedded/cef/raw/master/tools/automate/automate-git.py
    goto quit
)

if not exist %CHROMUIM_DOWNLOAD_DIR% mkdir %CHROMUIM_DOWNLOAD_DIR%

::================================================================

:: Set Visual Studio version used for building chromium: 2013 or 2015
set GYP_MSVS_VERSION=2013

:: Generate Visual Studio project for debugging chromium
set GYP_GENERATORS=ninja,msvs-ninja

:: Perform "official" build (disables debugging code and enables additional optimizations)
set GYP_DEFINES=buildtype=Official

::=====  x86 build  ==============================================

echo ----------- Updating chromium sources and tools neccesary for building chromium -----------
python automate-git.py ^
    --download-dir=%CHROMUIM_DOWNLOAD_DIR% ^
    --no-build ^
    --no-debug-build ^
    --no-distrib ^
    --branch=%CHROMIUM_BRANCH%

::================================================================

echo ----------- Registering patch for chromium -----------
copy /Y launch_win.patch %CHROMUIM_DOWNLOAD_DIR%\chromium\src\cef\patch\patches\launch_win.patch
copy /Y %CHROMUIM_DOWNLOAD_DIR%\chromium\src\cef\patch\patch.cfg .\temp.cfg
copy /Y /B temp.cfg /B +patch_config.in /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\cef\patch\patch.cfg /B
del /Q /F temp.cfg

::================================================================

echo ----------- Starting build process for x86 -----------
python automate-git.py ^
    --download-dir=%CHROMUIM_DOWNLOAD_DIR% ^
    --no-update ^
    --no-debug-build ^
    --minimal-distrib-only ^
    --force-build ^
    --branch=%CHROMIUM_BRANCH%

::================================================================

:: Reset changes in chromium repository before building x64
set CWD=%CD%
cd /D %CHROMUIM_DOWNLOAD_DIR%\chromium\src

git reset --hard
git clean -fd -f

cd /D %CWD%

::=====  x64 build  ==============================================

echo ----------- Updating chromium sources and tools neccesary for building chromium -----------
python automate-git.py ^
    --download-dir=%CHROMUIM_DOWNLOAD_DIR% ^
    --no-build ^
    --no-debug-build ^
    --no-distrib ^
    --x64-build ^
    --branch=%CHROMIUM_BRANCH%

::================================================================

echo ----------- Registering patch for chromium -----------
copy /Y launch_win.patch %CHROMUIM_DOWNLOAD_DIR%\chromium\src\cef\patch\patches\launch_win.patch
copy /Y %CHROMUIM_DOWNLOAD_DIR%\chromium\src\cef\patch\patch.cfg .\temp.cfg
copy /Y /B temp.cfg /B +patch_config.in /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\cef\patch\patch.cfg /B
del /Q /F temp.cfg

::================================================================

set GYP_DEFINES=buildtype=Official target_arch=x64

echo ----------- Starting build process x64 -----------
python automate-git.py ^
    --download-dir=%CHROMUIM_DOWNLOAD_DIR% ^
    --no-update ^
    --no-debug-build ^
    --minimal-distrib-only ^
    --force-build ^
    --x64-build ^
    --branch=%CHROMIUM_BRANCH%

::================================================================

echo ----------- Copy x86 binaries and other files to dava.engine home

mkdir %DAVA_ENGINE_HOME%\Tools\Bin\cef

copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\d3dcompiler_47.dll  %DAVA_ENGINE_HOME%\Tools\Bin\cef\d3dcompiler_47.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\libcef.dll          %DAVA_ENGINE_HOME%\Tools\Bin\cef\libcef.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\libEGL.dll          %DAVA_ENGINE_HOME%\Tools\Bin\cef\libEGL.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\libGLESv2.dll       %DAVA_ENGINE_HOME%\Tools\Bin\cef\libGLESv2.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\cef.pak             %DAVA_ENGINE_HOME%\Tools\Bin\cef\cef.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\cef_100_percent.pak %DAVA_ENGINE_HOME%\Tools\Bin\cef\cef_100_percent.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\cef_200_percent.pak %DAVA_ENGINE_HOME%\Tools\Bin\cef\cef_200_percent.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\cef_extensions.pak  %DAVA_ENGINE_HOME%\Tools\Bin\cef\cef_extensions.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\natives_blob.bin    %DAVA_ENGINE_HOME%\Tools\Bin\cef\natives_blob.bin
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\snapshot_blob.bin   %DAVA_ENGINE_HOME%\Tools\Bin\cef\snapshot_blob.bin
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\icudtl.dat          %DAVA_ENGINE_HOME%\Tools\Bin\cef\icudtl.dat

mkdir %DAVA_ENGINE_HOME%\Tools\Bin\cef\locales
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\locales\en-US.pak %DAVA_ENGINE_HOME%\Tools\Bin\cef\locales\en-US.pak

copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\libcef.dll.lib %DAVA_ENGINE_HOME%\Libs\lib_CMake\win\x86\Debug\libcef.lib
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release\libcef.dll.lib %DAVA_ENGINE_HOME%\Libs\lib_CMake\win\x86\Release\libcef.lib

::================================================================

echo ----------- Copy x64 binaries and other files to dava.engine home

mkdir %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef

copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\d3dcompiler_47.dll  %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\d3dcompiler_47.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\libcef.dll          %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\libcef.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\libEGL.dll          %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\libEGL.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\libGLESv2.dll       %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\libGLESv2.dll
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\cef.pak             %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\cef.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\cef_100_percent.pak %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\cef_100_percent.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\cef_200_percent.pak %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\cef_200_percent.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\cef_extensions.pak  %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\cef_extensions.pak
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\natives_blob.bin    %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\natives_blob.bin
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\snapshot_blob.bin   %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\snapshot_blob.bin
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\icudtl.dat          %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\icudtl.dat

mkdir %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\locales
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\locales\en-US.pak %DAVA_ENGINE_HOME%\Tools\Bin\x64\cef\locales\en-US.pak

copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\libcef.dll.lib %DAVA_ENGINE_HOME%\Libs\lib_CMake\win\x64\Debug\libcef.lib
copy /Y /B %CHROMUIM_DOWNLOAD_DIR%\chromium\src\out\Release_x64\libcef.dll.lib %DAVA_ENGINE_HOME%\Libs\lib_CMake\win\x64\Release\libcef.lib

::================================================================

:quit

:: Unset environment variables
set GYP_MSVS_VERSION=
set GYP_GENERATORS=
set GYP_DEFINES=

set CHROMIUM_BRANCH=
set CHROMUIM_DOWNLOAD_DIR=
set DAVA_ENGINE_HOME=
set CWD=
