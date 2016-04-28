
macro(grab_libs OUTPUT_LIST_VAR LIB_LIST EXCLUDE_LIBS ADDITIONAL_LIBS)
    set(OUTPUT_LIST "")
    foreach (LIB_FILE ${LIB_LIST})
        get_filename_component(LIB_NAME ${LIB_FILE} NAME)
        list (FIND ${EXCLUDE_LIBS} ${LIB_NAME} LIB_INDEX)
        if (${LIB_INDEX} EQUAL -1)
            list ( APPEND OUTPUT_LIST ${LIB_FILE}  )
        endif()
    endforeach()
    list (APPEND OUTPUT_LIST ${${ADDITIONAL_LIBS}})
    set(${OUTPUT_LIST_VAR} ${OUTPUT_LIST})
endmacro()


set( DAVA_STATIC_LIBRARIES_IOS      ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto_ios_release.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl_ios_release.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_ios.a
                                    )

set( DAVA_STATIC_LIBRARIES_MACOS    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libFColladaS.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libPVRTexLib.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto_macos_release.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpsd.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl_macos_release.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbis_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbisfile_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a 
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_macos.a 
                                    )

set( DAVA_STATIC_LIBRARIES_ANDROID  "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libzip_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua_android.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp_android.a"
                                    "-lEGL"
                                    "-lGLESv1_CM"
                                    "-llog"
                                    "-landroid"
                                    "-lGLESv2"
                                    "-lstdc++" 
                                    "-latomic" 
                                    )         



if( WIN32 AND NOT WINDOWS_UAP ) 

    set( WIN32_RELEASE "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/detours.lib"
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
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/lua_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/pnglib_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/PVRTexLib32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/TextureConverter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/unibreak_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/zlib.lib"   )

    set( WIN32_DEBUG   "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/detours.lib"
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
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/lua_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/pnglib_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/PVRTexLib32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/TextureConverterD.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/unibreak_wind.lib"  
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/zlib.lib" )

    set( WIN64_RELEASE "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/FColladaVS2010.lib"
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
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/lua_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/pnglib_win.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/PVRTexLib.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/ssleay32_64.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/TextureConverter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/theora_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/unibreak.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/uv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/z.lib" )

    set( WIN64_DEBUG   "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/freetype.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glut32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/icucommon.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/jpeg_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libcurl_a_debug.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libdxtd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libmongodb_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libogg_static.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libvorbisfile_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libvorbis_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libxml_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libyaml_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/lua_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/pnglib_wind.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/PVRTexLib32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/TextureConverter_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/theora_static_d.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/unibreak.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/z.lib" )


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

    if( X64_MODE )
        set( DAVA_STATIC_LIBRARIES_WIN32_RELEASE ${WIN64_RELEASE} )
        set( DAVA_STATIC_LIBRARIES_WIN32_DEBUG   ${WIN64_DEBUG} )

        list ( APPEND DAVA_STATIC_LIBRARIES_WIN32
            "$ENV{DXSDK_DIR}/lib/x64/d3dx9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3d9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3d11.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3dcompiler.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/dxguid.lib"
        )
    else ()
        set( DAVA_STATIC_LIBRARIES_WIN32_RELEASE ${WIN32_RELEASE} )
        set( DAVA_STATIC_LIBRARIES_WIN32_DEBUG   ${WIN32_DEBUG} )

        list ( APPEND DAVA_STATIC_LIBRARIES_WIN32

            "$ENV{DXSDK_DIR}/lib/x86/d3dx9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3d9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3d11.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3dcompiler.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/dxguid.lib"
        )
    endif ()

    list ( APPEND DAVA_STATIC_LIBRARIES_WIN32_RELEASE "msvcrt.lib"  )
    list ( APPEND DAVA_STATIC_LIBRARIES_WIN32_DEBUG   "msvcrtd.lib" )

endif()


if( WINDOWS_UAP ) 
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
