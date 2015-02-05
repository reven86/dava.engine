#
#set( MACOS_PLIST          )
#set( MACOS_ICO            )
#set( MACOS_DATA           )
#set( MACOS_DYLIB          )

#set( ANDROID_PACKAGE            )
#set( ANDROID_APP_NAME           )
#set( ANDROID_MIN_SDK_VERSION    )
#set( ANDROID_TARGET_SDK_VERSION )
#set( ANDROID_KEY_STORE_PATH     )#dava.config
#set( ANDROID_KEY_ALIAS_NAME     )#dava.config
#set( ANDROID_STORE_PASSWORD     )#dava.config
#set( ANDROID_ALIAS_PASSWORD     )#dava.config

#set( ADDED_SRC            )
#set( LIBRARIES            )
#set( LIBRARIES_RELEASE    )
#set( LIBRARIES_DEBUG      )
#set( ADDED_BINARY_DIR     )
#set( EXECUTABLE_FLAG      )
#

macro( setup_main_executable )

add_definitions ( -D_CRT_SECURE_NO_DEPRECATE )

if( APPLE )
    file ( GLOB_RECURSE RESOURCES_LIST ${MACOS_DATA} )
    foreach ( FILE ${RESOURCES_LIST} )
        get_filename_component ( FILE_PATH ${FILE} PATH ) 
        STRING(REGEX REPLACE "${CMAKE_CURRENT_LIST_DIR}/" "" FILE_GROUP ${FILE_PATH} )

        set_source_files_properties( ${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${FILE_GROUP} )
    endforeach ()
 
    file ( GLOB DYLIB_FILES    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.dylib)
    list ( APPEND DYLIB_FILES  ${MACOS_DYLIB} ) 

    set_source_files_properties( ${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks )

    list ( APPEND RESOURCES_LIST  ${DYLIB_FILES} ) 
    list ( APPEND RESOURCES_LIST  ${MACOS_PLIST} )
    list ( APPEND RESOURCES_LIST  ${MACOS_ICO} )
    list ( APPEND LIBRARIES       ${DYLIB_FILES} )

endif()


if( ANDROID )
    add_library( ${PROJECT_NAME} SHARED
        ${ADDED_SRC} 
        ${PROJECT_SOURCE_FILES} 
    )

else()                             
    add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${EXECUTABLE_FLAG}
        ${ADDED_SRC} 
        ${PROJECT_SOURCE_FILES} 
        ${RESOURCES_LIST}
    )

endif()



if( ANDROID )
    set( ANDROID_MIN_SDK_VERSION     ${ANDROID_NATIVE_API_LEVEL} )
    set( ANDROID_TARGET_SDK_VERSION  ${ANDROID_NATIVE_API_LEVEL} )

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/AndroidManifest.in
                    ${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml )

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/AntProperties.in
                    ${CMAKE_CURRENT_BINARY_DIR}/ant.properties )

    if( ANDROID_JAVA_SRC )
        foreach ( ITEM ${ANDROID_JAVA_SRC} )
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${ITEM} ${CMAKE_BINARY_DIR}/src )
        endforeach ()
    endif()

    if( ANDROID_JAVA_LIBS )
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${ANDROID_JAVA_LIBS} ${CMAKE_BINARY_DIR}/libs )
    endif()

    if( ANDROID_JAVA_RES )
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${ANDROID_JAVA_RES} ${CMAKE_BINARY_DIR}/res )
    endif()

    if( ANDROID_JAVA_ASSET )
        if( ANDROID_JAVA_ASSET_FOLDER )
            set( ASSETS_FOLDER "/${ANDROID_JAVA_ASSET_FOLDER}" )    
        endif()
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${ANDROID_JAVA_ASSET} ${CMAKE_BINARY_DIR}/assets${ASSETS_FOLDER} )
    endif()

    if( ANDROID_ICO )
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${ANDROID_ICO}  ${CMAKE_BINARY_DIR} )     
    endif()

    ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND  android update project --name ${ANDROID_APP_NAME} --target android-${ANDROID_TARGET_API_LEVEL} --path . 
        COMMAND  ant release
    )
        
    file ( GLOB SO_FILES ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.so )
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME} )
    foreach ( FILE ${SO_FILES} )
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${FILE}  ${CMAKE_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME} )
    endforeach ()

    set_target_properties( ${PROJECT_NAME} PROPERTIES IMPORTED_LOCATION ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/ )

elseif( MACOS )
    ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND   
        install_name_tool -change ./libfmodex.dylib    @executable_path/../Frameworks/libfmodex.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/Frameworks/libfmodevent.dylib  

        COMMAND   
        install_name_tool -change ./libfmodevent.dylib @executable_path/../Frameworks/libfmodevent.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}    

        COMMAND   
        install_name_tool -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}   

        COMMAND 
        install_name_tool -change ./libIMagickHelper.dylib @executable_path/../Frameworks/libIMagickHelper.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}     
    )


    set_target_properties ( ${PROJECT_NAME} PROPERTIES
                            MACOSX_BUNDLE_INFO_PLIST ${MACOS_PLIST} 
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                            RESOURCE "${MACOS_ICO}"
                          )
    set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )

elseif ( MSVC )
    if( ${EXECUTABLE_FLAG} STREQUAL "WIN32")
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:\"\" /NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"" ) 

    else()
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"" )    
    
    endif()

    if( DEBUG_INFO )   
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /SUBSYSTEM:WINDOWS" )
    else()
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS" )
    endif()

    list( APPEND DAVA_BINARY_WIN32_DIR "${ADDED_BINARY_DIR}" )
    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/DavaVcxprojUserTemplate.in
                    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user @ONLY )

    if( OUTPUT_TO_BUILD_DIR )
        set( OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR} )
        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
            set_target_properties ( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${OUTPUT_DIR} )
        endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
     endif()

endif()

file_tree_check( "${DAVA_FOLDERS}" )

target_link_libraries( ${PROJECT_NAME} ${LIBRARIES} ${DAVA_LIBRARY} )

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()


endmacro ()
