cmake_minimum_required (VERSION 2.8)

project      ( TestBed )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
find_package ( DavaFramework REQUIRED )

include      ( CMake-common )

if( ANDROID )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-invalid-offsetof" )

elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )

   # set_source_files_properties(  ${DAVA_TOOLS_DIR}/TeamcityOutput/TeamcityOutput.cpp PROPERTIES COMPILE_FLAGS "Objective-C++" )  

    set( CMAKE_IOS_SDK_ROOT Latest IOS )
    set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -stdlib=libc++ -g -Wall")

    file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/iOSSpecific/*.xib" )

    list( APPEND RESOURCES_LIST ${XIB_LIST} )
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TestBed.plist  )

elseif( MACOS )

    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -stdlib=libc++ -g -Wall")
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )

    file ( GLOB_RECURSE RESOURCES_LIST "${CMAKE_CURRENT_LIST_DIR}/Data/*" )
    foreach ( FILE ${RESOURCES_LIST} )

        get_filename_component ( FILE_PATH ${FILE} PATH ) 
        STRING(REGEX REPLACE "${CMAKE_CURRENT_LIST_DIR}/" "" FILE_GROUP ${FILE_PATH} )

        set_source_files_properties( ${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${FILE_GROUP} )
    endforeach ()

    file ( GLOB DYLIB_FILES ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.dylib)
    set_source_files_properties( ${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks )

    file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/*.xib" )

    list( APPEND RESOURCES_LIST ${XIB_LIST} )
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TemplateProjectMacOS-Info.plist )

    list ( APPEND RESOURCES_LIST  ${DYLIB_FILES} ) 
    list ( APPEND LIBRARIES       ${DYLIB_FILES} )

elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    
endif()

include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

add_definitions       ( -D_CRT_SECURE_NO_DEPRECATE )

set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -DDAVA_DEBUG" )
set( TeamcityOutput          ${DAVA_TOOLS_DIR}/TeamcityOutput  )
set( PackerTools             ${DAVA_TOOLS_DIR}/TexturePacker )

set( PackerToolsSCR          ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.h 
                             ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.cpp )

define_source_folders ( SRC_ROOT "Classes" ${TeamcityOutput}  ${PLATFORM_SPECIFIC_FOLDER} 
                        GLOB_ERASE_FOLDERS "_build"  )

generate_source_groups_project ( ROOT_DIR "Classes" ${PLATFORM_SPECIFIC_FOLDER}  ${PLATFORM_SPECIFIC_FOLDER}  
                                 GROUP_FOLDERS TeamcityOutput PackerTools )

if( ANDROID )  
    add_library( ${PROJECT_NAME} SHARED 
        ${PROJECT_SOURCE_FILES}  ${PackerToolsSCR} )

else()                             
    add_executable  ( ${PROJECT_NAME} MACOSX_BUNDLE WIN32
        ${PROJECT_SOURCE_FILES} 
        ${RESOURCES_LIST} ${PackerToolsSCR} )

endif()


if( ANDROID )

    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/android/AndroidManifest.xml ${CMAKE_BINARY_DIR} )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/Java ${CMAKE_BINARY_DIR}/src )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/android/src ${CMAKE_BINARY_DIR}/src )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/android/res ${CMAKE_BINARY_DIR}/res )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/jar ${CMAKE_BINARY_DIR}/libs )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Data ${CMAKE_BINARY_DIR}/assets/Data )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png  ${CMAKE_BINARY_DIR} )     
    
    file ( GLOB SO_FILES ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.so )

    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME} )
    foreach ( FILE ${SO_FILES} )
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${FILE}  ${CMAKE_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME} )
    endforeach ()

    set_target_properties( ${PROJECT_NAME} PROPERTIES IMPORTED_LOCATION ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/ )

elseif( IOS )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TestBed.plist 
                            RESOURCE "${XIB_LIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                          )

    get_target_property ( TARGET_LOC ${PROJECT_NAME} LOCATION )
    string ( REGEX REPLACE /Contents/MacOS/${PROJECT_NAME} "" TARGET_LOC ${TARGET_LOC} )
    add_custom_command( TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Data ${TARGET_LOC}/Data
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TestBed.plist ${CMAKE_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/Info.plist
    )

elseif( MACOS )    
    set( BUILD_MODE $<CONFIG> ) #$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release> )

    ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND   
        install_name_tool -change ./libfmodex.dylib    @executable_path/../Frameworks/libfmodex.dylib ${CMAKE_BINARY_DIR}/${BUILD_MODE}/${PROJECT_NAME}.app/Contents/Frameworks/libfmodevent.dylib  

        COMMAND   
        install_name_tool -change ./libfmodevent.dylib @executable_path/../Frameworks/libfmodevent.dylib ${CMAKE_BINARY_DIR}/${BUILD_MODE}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}    

        COMMAND   
        install_name_tool -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib ${CMAKE_BINARY_DIR}/${BUILD_MODE}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}    
    )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TemplateProjectMacOS-Info.plist
                            RESOURCE "${XIB_LIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                          )


elseif( WIN32 )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"" )    
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS" )
    list( APPEND DAVA_BINARY_WIN32_DIR "${ADDED_BINARY_DIR}" )
    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/DavaVcxprojUserTemplate.in
                    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user @ONLY )
    
endif()


target_link_libraries( ${PROJECT_NAME}
    ${DAVA_LIBRARY}
    ${LIBRARIES}
)

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()





