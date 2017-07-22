
set( DAVA_STATIC_LIBRARIES_IOS      ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3_ios.a
                                    )

set( DAVA_STATIC_LIBRARIES_MACOS    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libFColladaS.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libPVRTexLib.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpsd.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbis_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbisfile_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a 
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3.a
                                    )

set( DAVA_STATIC_LIBRARIES_ANDROID  "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libzip.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3.a"
                                    "-lEGL"
                                    "-lGLESv1_CM"
                                    "-llog"
                                    "-landroid"
                                    "-lGLESv2"
                                    "-latomic" 
                                    )

set( DAVA_STATIC_LIBRARIES_LINUX
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbis.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbisfile.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a"
                                    "-ldl"
                                    "-lz"
                                    )
                                    
if( DEFINED ANDROID_NDK 
	AND DEFINED ANDROID_STL_PREFIX 
	AND DEFINED ANDROID_ABI 
	AND ANDROID_STL STREQUAL c++_shared)
# Add c++abi lib for c++_shared STL
set( DAVA_STATIC_LIBRARIES_ANDROID ${DAVA_STATIC_LIBRARIES_ANDROID}
                                   ${ANDROID_NDK}/sources/cxx-stl/${ANDROID_STL_PREFIX}/libs/${ANDROID_ABI}/libc++abi.a 
                                   )
endif()

if( WIN ) 

    set( DAVA_STATIC_LIBRARIES_WIN32_RELEASE
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/detours.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/freetype246MT.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glut32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glutstatic.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/icucommon.lib"         
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libcurl.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libdxt.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libeay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libjpeg.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libmongodb_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libogg_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libtheora_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libvorbisfile_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libvorbis_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libxml_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libyaml_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/pnglib_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/PVRTexLib32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/TextureConverter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/unibreak_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/zlib.lib"   
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/sqlite3.lib"   )

    set( DAVA_STATIC_LIBRARIES_WIN32_DEBUG
                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/detours.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/freetype246MT_D.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glut32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glutstatic.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/icucommon.lib"         
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libcurl.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libdxtd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libeay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libjpegd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libmongodb_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libogg_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libtheora_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libvorbisfile_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libvorbis_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libxml_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libyaml_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/pnglib_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/PVRTexLib32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/TextureConverterD.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/unibreak_wind.lib"  
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/zlib.lib" 
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/sqlite3.lib" )

    set( DAVA_STATIC_LIBRARIES_WIN64_RELEASE
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/freetype.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glut32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/icucommon.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/jpeg.lib"              
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libcurl_a.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libdxt.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libeay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libmongodb_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libogg_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libvorbisfile_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libvorbis_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libwebpdecoder.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libxml_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libyaml_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/pnglib_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/PVRTexLib.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/ssleay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/TextureConverter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/theora_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/unibreak.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/uv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/z.lib" 
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/sqlite3.lib" )

    set( DAVA_STATIC_LIBRARIES_WIN64_DEBUG
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libeay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/ssleay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/freetype.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glut32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/icucommon.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/jpeg_d.lib"            
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libcurl_a_debug.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libeay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libdxtd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libeay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libmongodb_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libogg_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libvorbisfile_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libvorbis_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libxml_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libyaml_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/pnglib_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/PVRTexLib32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/ssleay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/TextureConverter_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/theora_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/unibreak.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/z.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/sqlite3.lib")


    set( DAVA_STATIC_LIBRARIES_WIN32 "Wininet.lib"
                                     "opengl32.lib"
                                     "ws2_32.lib"
                                     "winmm.lib"
                                     "wldap32.lib"
                                     "iphlpapi.lib"
                                     "Gdi32.lib"
                                     "glu32.lib"
                                     "psapi.lib"
                                     "dbghelp.lib"
                                     "userenv.lib"
                                     "delayimp.lib"
                                     "dxgi.lib" )

    set( DAVA_DYNAMIC_LIBRARIES_WIN32 "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avcodec-57.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avdevice-57.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avfilter-6.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avformat-57.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avutil-55.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/postproc-54.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swresample-2.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swscale-4.dll" 
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/D3DCompiler_43.dll" 
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/d3dx9_43.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glew32.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/msvcr120.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/msvcp120.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/TextureConverter.dll")

    set( DAVA_DYNAMIC_LIBRARIES_WIN64 ${DAVA_DYNAMIC_LIBRARIES_WIN32} )


    set ( DAVA_STATIC_LIBRARIES_WIN64 ${DAVA_STATIC_LIBRARIES_WIN32} )

    list ( APPEND DAVA_STATIC_LIBRARIES_WIN32
            "$ENV{DXSDK_DIR}/lib/x86/d3dx9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3d9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3d11.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3dcompiler.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/dxguid.lib"
        )

    list ( APPEND DAVA_STATIC_LIBRARIES_WIN64
            "$ENV{DXSDK_DIR}/lib/x64/d3dx9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3d9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3d11.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3dcompiler.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/dxguid.lib"
        )

endif()

if( WINUAP ) 
    add_static_libs_win_uap ( "${DAVA_WIN_UAP_LIBRARIES_PATH_COMMON}" LIST_SHARED_LIBRARIES )

    set( DAVA_STATIC_LIBRARIES_WINUAP   "d2d1.lib"
                                        "d3d11.lib"
                                        "d3dcompiler.lib"
                                        "dxgi.lib"
                                        "dxguid.lib"
                                        "dwrite.lib"
                                        "iphlpapi.lib" 
                                        ${LIST_SHARED_LIBRARIES} )
                                        
    set( DAVA_STATIC_LIBRARIES_WINUAP_RELEASE ${LIST_SHARED_LIBRARIES_RELEASE} )
    set( DAVA_STATIC_LIBRARIES_WINUAP_DEBUG   ${LIST_SHARED_LIBRARIES_DEBUG} )

endif()
