make clean
./configure --prefix=$DESTROOT --host=arm-apple-darwin --enable-static=yes --enable-shared=no CC="$ARM_CC" AR="$ARM_AR" LDFLAGS="$ARM_LDFLAGS" CFLAGS="$ARM_CFLAGS"
make