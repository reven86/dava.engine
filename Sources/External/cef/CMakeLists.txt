set( NAME_MODULE CEFWebview )
set( MODULE_TYPE STATIC )

set( HPP_FILES Sources/*.h Sources/*.hpp )
set( CPP_FILES Sources/*.cpp     )

set ( UNITY_IGNORE_LIST "Sources/CEFController.cpp"
                        "Sources/CEFWebViewControl.cpp"
                        "Sources/CEFWebViewControlProxy.cpp"
                        "Sources/CEFWebPageRender.cpp"
                        "Sources/CEFDavaResourceHandler.cpp" )

set( DEFINITIONS      -DENABLE_CEF_WEBVIEW -DDISABLE_NATIVE_WEBVIEW )
set( INCLUDES_PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include )

set ( CEF_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Libs/win )

set( STATIC_LIBRARIES_WIN32_RELEASE  ${CEF_LIBS_DIR}/x86/Release/libcef.lib 
                                     ${CEF_LIBS_DIR}/x86/Release/libcef_dll_wrapper.lib )

set( STATIC_LIBRARIES_WIN32_DEBUG    ${CEF_LIBS_DIR}/x86/Debug/libcef.lib 
                                     ${CEF_LIBS_DIR}/x86/Debug/libcef_dll_wrapper.lib )
                                     
set( STATIC_LIBRARIES_WIN64_RELEASE  ${CEF_LIBS_DIR}/x64/Release/libcef.lib 
                                     ${CEF_LIBS_DIR}/x64/Release/libcef_dll_wrapper.lib )

set( STATIC_LIBRARIES_WIN64_DEBUG    ${CEF_LIBS_DIR}/x64/Debug/libcef.lib 
                                     ${CEF_LIBS_DIR}/x64/Debug/libcef_dll_wrapper.lib )

if ( X64_MODE )
    set( DEPLOY_TO_BIN_WIN "${CMAKE_CURRENT_SOURCE_DIR}/Data/x64/*" )
    set( BINARY_WIN32_DIR_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Data/x64" )
    file(GLOB RELEASE_LIBS "${CEF_LIBS_DIR}/x64/NGT/Release/*.lib")
    file(GLOB DEBUG_LIBS   "${CEF_LIBS_DIR}/x64/NGT/Debug/*.lib")
else()
    set( DEPLOY_TO_BIN_WIN "${CMAKE_CURRENT_SOURCE_DIR}/Data/x86/*" )
    set( BINARY_WIN32_DIR_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Data/x86" )
    file(GLOB RELEASE_LIBS "${CEF_LIBS_DIR}/x86/NGT/Release/*.lib")
    file(GLOB DEBUG_LIBS   "${CEF_LIBS_DIR}/x86/NGT/Debug/*.lib")
endif ()

set(EXCLUDE_LIBS "libcef_dll_wrapper.lib" CACHE INTERNAL "")
set(ADDITIONAL_RELEASE_LIBS ${RELEASE_LIBS} CACHE INTERNAL "")
set(ADDITIONAL_DEBUG_LIBS ${DEBUG_LIBS} CACHE INTERNAL "")

setup_main_module()
