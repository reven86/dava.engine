#!/bin/sh
#Our short version of steam_sdk takan from https://partner.steamgames.com
#You should call this script to prepare libs for dava

install_name_tool -id @rpath/libsteam_api.dylib redistributable_bin/osx32/libsteam_api.dylib
install_name_tool -id @rpath/libsdkencryptedappticket.dylib public/steam/lib/osx32/libsdkencryptedappticket.dylib