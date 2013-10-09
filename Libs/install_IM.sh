#!/bin/bash
#
# Mac OS: It is strongly recommended that the X11R6 package be installed since 
# this enables ImageMagick's X11 support (animate, display, and import will work) 
# and it includes the Freetype v2 DLL required to support TrueType and Postscript Type 1 fonts. 
#
# Make sure that /usr/X11R6/bin is in your PATH prior to running configure.
#

#in case of the problems with png convertion occures macPorts should be installed and line "./configure ..." must be updated to LDFLAGS="-L$currPath/$installPath/delegates/lib -L/opt/local/lib"

type -p X &>/dev/null && echo 'X11 is installed' || echo 'WARNING: X11 is not detected. It is strongly recommended that the X11R6 package be installed since this enables ImageMagick X11 support (animate, display, and import will work). Make sure that /usr/X11R6/bin is in your PATH.'

installPath="ImageMagick-6.7.4"
imPath="ImageMagick-6.7.4-10"

rm -rf $imPath
rm -rf $installPath

tar -xf $imPath.tar.gz

currPath=`pwd`

mkdir -p $currPath/$installPath/delegates
mkdir $currPath/$installPath/delegates/lib
mkdir $currPath/$installPath/delegates/include

cp $currPath/libs/libpng_macos.a $currPath/$installPath/delegates/lib/libpng-static.a
cp $currPath/include/libpng/*.h $currPath/$installPath/delegates/include/

cd $imPath

./configure --prefix=$currPath/$installPath --disable-shared --without-dps --without-djvu --without-tiff --without-jpeg --without-fontconfig --without-gslib --without-gvc --without-lcms --without-lcms2 --without-lqr --without-lzma --without-openexr --without-rsvg --without-webp --without-wmf --without-xml --disable-openmp --disable-opencl --with-x=no --with-png=yes CPPFLAGS=-I$currPath/$installPath/delegates/include LDFLAGS=-L$currPath/$installPath/delegates/lib
make
make install
cd ..

rm -rf $imPath

