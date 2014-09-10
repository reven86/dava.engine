cd ..\Release

if exist app rmdir /s /q app
if exist app.zip del /q app.zip

mkdir app\Data\
xcopy /e ..\Data\*.* app\Data\ 
xcopy UnitTestsVS2010.exe app\
xcopy ..\..\..\Libs\glew\bin\glew32.dll app\
xcopy ..\..\..\Libs\fmod\bin\fmodex.dll app\
xcopy ..\..\..\Libs\fmod\bin\fmod_event.dll app\

wzzip -p -r UnitTestsWin.zip app\*.*