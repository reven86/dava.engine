Header files for all platforms are essentially the same, they may differ in comment blocks.
So use header files from fmod installation for win32 and platform specific files like fmodiphone.h,
fmodwindowsstoreapp.h.

====================================================================================================

Due to nature of macOs dynamic loader we should change fmod dylib's identification names to tell
loader to search dynamic libraries in runpaths specified when building executable.

install_name_tool -id @rpath/libfmodex.dylib ./libfmodex.dylib
install_name_tool -id @rpath/libfmodevent.dylib ./libfmodevent.dylib
install_name_tool -change ./libfmodex.dylib @rpath/libfmodex.dylib ./libfmodevent.dylib

install_name_tool -id @rpath/libfmodexL.dylib ./libfmodexL.dylib
install_name_tool -id @rpath/libfmodeventL.dylib ./libfmodeventL.dylib
install_name_tool -change ./libfmodexL.dylib @rpath/libfmodexL.dylib ./libfmodeventL.dylib

Another option is to place fmod dylibs at the same level in bundle as executable, i.e. at Contents/MacOS.
