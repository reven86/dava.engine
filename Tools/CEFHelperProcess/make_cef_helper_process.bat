::clean build directory
rmdir /s /q _build
mkdir _build
cd _build

::generate project and build
cmake -G"Visual Studio 12" .. && cmake --build . --config Release

::leave directory and copy artifacts to Tools/Bin/cef
cd ..
copy /Y _build\Release\CEFHelperProcess.exe ..\Bin\cef