::clean build directory
rmdir /s /q _build
rmdir /s /q _build_x64
mkdir _build
mkdir _build_x64

::generate project and build
cd _build && cmake -G"Visual Studio 12" .. -DUNITY_BUILD=true && cmake --build . --config Release && cd ..
cd _build_x64 && cmake -G"Visual Studio 12 Win64" .. -DUNITY_BUILD=true && cmake --build . --config Release && cd ..

::leave directory and copy artifacts to Tools/Bin/cef
copy /Y _build\Release\CEFHelperProcess.exe ..\Bin\cef
copy /Y _build_x64\Release\CEFHelperProcess.exe ..\Bin\x64\cef