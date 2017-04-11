Header files for all platforms are essentially the same, they may differ in comment blocks.
So use header files from fmod installation for win32 and platform specific files like fmodiphone.h,
fmodwindowsstoreapp.h.

====================================================================================================

Windows 10 fmod package contains libraries with names fmodexUWP_vc.lib, fmodexUWP64_vc.lib, fmodexUWParm_vc.lib, etc.
Our cmake scripts expect library names to be the same across all platforms, so when updating fmod rename libraries:
    fmodexUWP_vc, fmod_eventUWP for Release configuration
    fmodexUWPD_vc, fmod_eventUWPD for Debug configuration

====================================================================================================

Do not forget to update fmodex.jar in:
    dava.framework/Modules/Sound/Libs/Android/jar
    dava.framework/Programs/AndroidFramework/libs

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
