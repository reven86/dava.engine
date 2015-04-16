
#compiller flags
if( NOT DISABLE_DEBUG )
    set( CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} -D__DAVAENGINE_DEBUG__" )

endif  ()

if     ( ANDROID )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wno-invalid-offsetof" )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -mfloat-abi=softfp -mfpu=neon -Wno-invalid-offsetof -frtti" )    
    set( CMAKE_ECLIPSE_MAKE_ARGUMENTS -j8 )
    
elseif ( IOS     ) 
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fvisibility=hidden" )
    set( CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS} -O0" )
    set( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} -O2" )
    
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )
    set( CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY iPhone/iPad )
    set( CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 7.0 )

    set( CMAKE_IOS_SDK_ROOT Latest IOS )
    set( CMAKE_OSX_ARCHITECTURES armv7 armv7s i386 arm64 )

    if( NOT IOS_BUNDLE_IDENTIFIER )
        set( IOS_BUNDLE_IDENTIFIER com.davaconsulting.${PROJECT_NAME} )
        
    endif()
    
    # Fix try_compile
    set( MACOSX_BUNDLE_GUI_IDENTIFIER  ${IOS_BUNDLE_IDENTIFIER} )
    set( CMAKE_MACOSX_BUNDLE YES )
    set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" )

elseif ( MACOS )
    set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )
    set( CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS YES )

elseif ( WIN32 )
    #dynamic runtime on windows store
    if (CMAKE_SYSTEM_NAME STREQUAL WindowsStore OR CMAKE_SYSTEM_NAME STREQUAL WindowsPhone)
	    set ( WINSTORE 1 )
	    set ( CRT_TYPE_DEBUG "/MDd" )
		set ( CRT_TYPE_RELEASE "/MD" )
		#consume windows runtime extension (C++/CX)
		set ( ADDITIONAL_CXX_FLAGS "/ZW")
	else ()
	    set ( CRT_TYPE_DEBUG "/MTd" )
		set ( CRT_TYPE_RELEASE "/MT" )
	endif ()
	
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CRT_TYPE_DEBUG} ${ADDITIONAL_CXX_FLAGS} /MP /EHsc" ) 
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CRT_TYPE_RELEASE} ${ADDITIONAL_CXX_FLAGS} /MP /EHsc" ) 
    set ( CMAKE_EXE_LINKER_FLAGS_RELEASE "/ENTRY:mainCRTStartup" )

    # undef macros min and max defined in windows.h
    add_definitions ( -DNOMINMAX )
endif  ()


##

if( WARNINGS_AS_ERRORS )
    if( ANDROID )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror" ) # warnings as errors

    elseif( APPLE )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror" ) # warnings as errors
        set( CMAKE_XCODE_ATTRIBUTE_GCC_TREAT_WARNINGS_AS_ERRORS  YES )

    elseif( WIN32 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX" )

    endif()

endif()


##
if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/ios" ) 
  
elseif ( MACOS )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/mac" ) 

elseif ( WIN32)
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win" ) 
    
endif  ()
