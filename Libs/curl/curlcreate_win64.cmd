#1. Copy this cmd-file into $(curl-dir)/winbuild/
#1. Run VS command promt
#2. go to $(curl-dir)/winbuild/
#3. Run this cmd
#4. Copy builds/libcurl-%buildtype%/libcurl_a(_debug).lib file to %frameworkpath%/Libs/libs/libcurl.lib

nmake /f ./winbuild/Makefile.vc mode=static RTLIBCFG=static VC=12 ENABLE_WINSSL=yes GEN_PDB=no DEBUG=no MACHINE=x64 ENABLE_THREADED_RESOLVER=yes