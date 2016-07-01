::clean build directory
rmdir /s /q _build
mkdir _build
cd _build

::generate project and build
cmake -G"Visual Studio 12" .. && cmake --build . --config Release

::leave directory and copy artifacts to Tools/Bin/cef
cd ..
copy /Y _build\Release\CEFHelperProcess.exe ..\Bin\cef

:: x64 build
rmdir /s /q _build64
mkdir _build64
cd _build64

cmake -G"Visual Studio 12 Win64" -DX64_MODE=1 .. && cmake --build . --config Release

cd ..
copy /Y _build64\Release\CEFHelperProcess.exe ..\Bin\x64\cef
