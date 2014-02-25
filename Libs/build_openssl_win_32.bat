rem run this script from VC command prompt

PUSHD openssl

rem build win32_release
rmdir tmp32 /s/q
call perl Configure VC-WIN32 --prefix=..\build_openssl_win32
call ms\do_ms
call nmake -f ms\nt.mak
call nmake -f ms\nt.mak install

rem build win32_debug
rem rmdir tmp32 /s/q
rem call perl Configure debug-VC-WIN32 --prefix=..\build_openssl_win32_dbg
rem call ms\do_ms
rem call nmake -f ms\nt_debug.mak
rem call nmake -f ms\nt_debug.mak install

POPD

rem move libs
move build_openssl_win32\lib\libeay32.lib libs\libeay32.lib
move build_openssl_win32\lib\ssleay32.lib libs\ssleay32.lib

rem clear temp files
rmdir openssl/tmp32 /q/s
rmdir build_openssl_win32 /q/s