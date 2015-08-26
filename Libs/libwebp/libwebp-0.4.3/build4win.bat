:: Building for MSVS 2015 x86 arch
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
nmake -f Makefile.vc CFG=debug-static ARCH=x86 OBJDIR=_build2015
nmake -f Makefile.vc CFG=release-static ARCH=x86 OBJDIR=_build2015
endlocal

:: Building for MSVS 2015 x64 arch
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
nmake -f Makefile.vc CFG=debug-static ARCH=x64 OBJDIR=_build2015
nmake -f Makefile.vc CFG=release-static ARCH=x64 OBJDIR=_build2015
endlocal

:: Building for MSVS 2015 arm arch
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_arm
nmake -f Makefile.vc CFG=debug-static ARCH=arm OBJDIR=_build2015
nmake -f Makefile.vc CFG=release-static ARCH=arm OBJDIR=_build2015
endlocal

:: Building for MSVS 2013 x86 arch
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
nmake -f Makefile.vc CFG=debug-static ARCH=x86 RTLIBCFG=static OBJDIR=_build2013
nmake -f Makefile.vc CFG=release-static ARCH=x86 RTLIBCFG=static OBJDIR=_build2013
endlocal
