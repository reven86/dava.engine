cmake_minimum_required (VERSION 2.8)
project      ( UnitTest )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

find_package ( DavaFramework REQUIRED )

include      ( CMake-common )


if( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )

    set( CMAKE_IOS_SDK_ROOT Latest IOS )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")

    file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/iOSSpecific/*.xib" )

  	list( APPEND RESOURCES_LIST ${XIB_LIST} )
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectiPhone-Info.plist )

elseif( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )

    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")

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
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectMacOS-Info.plist )

    list ( APPEND RESOURCES_LIST  ${DYLIB_FILES} ) 
    list ( APPEND LIBRARIES       ${DYLIB_FILES} )
    
endif()


include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

add_definitions( -D_CRT_SECURE_NO_DEPRECATE )

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDAVA_DEBUG" )


define_source_folders ( SRC_ROOT "Classes" ${PLATFORM_SPECIFIC_FOLDER} GLOB_ERASE_FOLDERS "_build" "Unix" )
generate_source_groups_project ()
                               
add_executable  ( UnitTest MACOSX_BUNDLE
    ${PROJECT_SOURCE_FILES} 
    ${RESOURCES_LIST}
)

target_link_libraries( UnitTest
    ${DAVA_LIBRARY}
    ${LIBRARIES}
)

if( IOS )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectiPhone-Info.plist 
                            RESOURCE "${XIB_LIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                          )

    get_target_property ( TARGET_LOC ${PROJECT_NAME} LOCATION )
    string ( REGEX REPLACE /Contents/MacOS/${PROJECT_NAME} "" TARGET_LOC ${TARGET_LOC} )
    add_custom_command( TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Data ${TARGET_LOC}/Data
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectiPhone-Info.plist ${CMAKE_BINARY_DIR}/CMakeFiles/UnitTest.dir/Info.plist
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

    set_target_properties ( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectMacOS-Info.plist
                            RESOURCE "${XIB_LIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                          )


endif()

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( UnitTest debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( UnitTest optimized ${FILE} )
endforeach ()





