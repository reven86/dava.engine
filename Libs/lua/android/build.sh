#!/bin/sh

echo "*****************   START   ****************"
cd `dirname $0`

ndk-build -j 2 NDK_DEBUG=0
if [ $? != 0 ]; then
echo "ERROR: Can't build test program!"
exit 1
fi

export PROJECT_DIR=`pwd`
cp $PROJECT_DIR/obj/local/armeabi-v7a/liblua_android.a $PROJECT_DIR/../../libs/

echo "*****************   FINISH   ****************"
