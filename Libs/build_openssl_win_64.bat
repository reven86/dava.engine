rem run this script from VC command prompt

PUSHD openssl

rem build win64_release
rmdir tmp32 /s/q
call perl Configure VC-WIN64A --prefix=..\build_openssl_win64
call ms\do_win64a
call nmake -f ms\nt.mak
call nmake -f ms\nt.mak install

rem build win64_debug
rem call perl Configure debug-VC-WIN64A --prefix=..\build_openssl-win64-dbg
rem call ms\do_win64a
rem call nmake -f ms\nt_debug.mak
rem call nmake -f ms\nt_debug.mak install

POPD

rem move libs
move build_openssl_win64\lib\libeay32.lib libs\libeay32_64.lib
move build_openssl_win64\lib\ssleay32.lib libs\ssleay32_64.lib

rem clear temp files
rmdir openssl/tmp32 /q/s
rmdir build_openssl_win64 /q/s