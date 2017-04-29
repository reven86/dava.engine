cmake_minimum_required (VERSION 3.0)

project      ( PerformanceTests )

set          ( WARNINGS_AS_ERRORS true )
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include      ( CMake-common )

if ( ENABLE_MEM_PROFILING )
    dava_add_definitions( -DDAVA_MEMORY_PROFILING_ENABLE )
endif()

set ( DAVA_COREV2 1 )
dava_add_definitions       (
-D__DAVAENGINE_COREV2__
)

find_package( DavaFramework REQUIRED COMPONENTS DAVA_DISABLE_AUTOTESTS "Sound" )

include_directories   ( "iOSSpecific" )
include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )
include_directories   ( "Classes/Controller" )
include_directories   ( "Classes/Screen" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

set( PackerTools             ${DAVA_TOOLS_DIR}/TexturePacker )
set( CommandLineTools        ${DAVA_TOOLS_DIR}/CommandLine/CommandLineParser.h 
                             ${DAVA_TOOLS_DIR}/CommandLine/CommandLineParser.cpp )

if( ANDROID )
    set( PLATFORM_SPECIFIC_FOLDER ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

elseif( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )
    file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/*.xib" )

elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )

elseif( WIN32 )
    set( EXECUTABLE_FLAG WIN32 )
endif()

define_source ( SOURCE "Classes" ${PLATFORM_SPECIFIC_FOLDER} ${TeamcityOutput} )

set( APP_DATA             ${CMAKE_CURRENT_LIST_DIR}/Data )

if (NOT PERFORMANCE_TEST_DATA_SVN)
    set( PERFORMANCE_TEST_DATA_SVN ${DAVA_ROOT_DIR}/../performance.test)
endif()

set( MACOS_XIB            ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/MainMenu.xib )
set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/icon.icns  )

set( IOS_PLISTT           ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )

set( ANDROID_USE_STANDART_TEMLATE true )
set( ANDROID_PACKAGE            "com.dava.performancetests" )
set( ANDROID_APP_NAME           "PerformanceTestApp" )
set( ANDROID_ACTIVITY_APP_NAME  "PerformanceTests"   )

set( ADDED_SRC            ${IOS_ADD_SRC})
set( LIBRARIES            )
set( LIBRARIES_RELEASE    )
set( LIBRARIES_DEBUG      )
set( ADDED_BINARY_DIR     )

if( NOT DAVA_MEGASOLUTION )

    if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        set( RES_EDITOR_BINARY   "${RES_EDITOR_PATH}/ResourceEditor.exe")       
    else()            
        set( RES_EDITOR_BINARY   "${RES_EDITOR_PATH}/ResourceEditor.app/Contents/MacOS/ResourceEditor")
    endif()

    set (TOOLS_DATA ${DAVA_ROOT_DIR}/Programs/Data)

    message( "--Performance test data folder: " ${PERFORMANCE_TEST_DATA_SVN} )

    if( NOT EXISTS ${PERFORMANCE_TEST_DATA_SVN})
        message( "Error !!!: Please checkout performance.test"  )
        message( " " )
        exit()
    endif()

    if( NOT RES_EDITOR_PATH )
        message( "Error !!!: Please set the correct path 'RES_EDITOR_PATH' to Resource editor in file DavaConfig.in"  )
        message( " " )
        exit()
    endif()

    if( NOT GPU_PARAM)
        set(GPU_PARAM   "PowerVR_iOS")
    endif()


    configure_file( ${CMAKE_CURRENT_LIST_DIR}/scripts/TemplateConvert3D.in
                        ${CMAKE_CURRENT_LIST_DIR}/DataSource/convert_3d.py @ONLY )

    configure_file( ${CMAKE_CURRENT_LIST_DIR}/scripts/TemplateConvert3D_FX.in
                        ${CMAKE_CURRENT_LIST_DIR}/DataSource/convert_3d_FX.py @ONLY )

    configure_file( ${CMAKE_CURRENT_LIST_DIR}/scripts/TemplateConvert3DTanks.in
                        ${CMAKE_CURRENT_LIST_DIR}/DataSource/convert_3d_tanks.py @ONLY )

    message( "============================ Copy resources ============================"  )

    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${TOOLS_DATA}/Materials ${APP_DATA}/Materials )

    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PERFORMANCE_TEST_DATA_SVN}/Data ${APP_DATA} )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PERFORMANCE_TEST_DATA_SVN}/DataSource ${APP_DATA}/../DataSource )

    message( "============================Convert resources ============================"  )

    execute_process( COMMAND python convert_everything.py --gpu ${GPU_PARAM} WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/DataSource )
    execute_process( COMMAND python create_file_structure.py WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/DataSource )

endif()

if (ANDROID)
    # Libraries to load at startup
    set (ANDROID_BOOT_MODULES "crystax;c++_shared;fmodex;fmodevent;PerformanceTests")
endif()

setup_main_executable()
