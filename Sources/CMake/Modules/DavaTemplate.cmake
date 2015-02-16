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

#set( ADDED_SRC                  )
#set( LIBRARIES                  )
#set( LIBRARIES_RELEASE          )
#set( LIBRARIES_DEBUG            )
#set( ADDED_BINARY_DIR           )
#set( EXECUTABLE_FLAG            )
#set( FILE_TREE_CHECK_FOLDERS    )
#

macro( setup_main_executable )

add_definitions ( -D_CRT_SECURE_NO_DEPRECATE )
 
if( IOS )
    list( APPEND RESOURCES_LIST ${MACOS_DATA} )    
    list( APPEND RESOURCES_LIST ${IOS_XIB} )
    list( APPEND RESOURCES_LIST ${IOS_PLIST} )
    list( APPEND RESOURCES_LIST ${IOS_ICO} )

elseif( MACOS )
    file ( GLOB DYLIB_FILES    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.dylib)
    
    set_source_files_properties( ${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

    list ( APPEND DYLIB_FILES     "${DYLIB_FILES}" "${MACOS_DYLIB}" )  

    list( APPEND RESOURCES_LIST  ${MACOS_DATA}  )
    list( APPEND RESOURCES_LIST  ${DYLIB_FILES} ) 
    list( APPEND RESOURCES_LIST  ${MACOS_XIB}   )    
    list( APPEND RESOURCES_LIST  ${MACOS_PLIST} )
    list( APPEND RESOURCES_LIST  ${MACOS_ICO}   )

    list( APPEND LIBRARIES      ${DYLIB_FILES} )

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
    set( LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME}" CACHE PATH "Output directory for Android libs" )

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
      
    file ( GLOB SO_FILES ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.so )
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME} )
    foreach ( FILE ${SO_FILES} )
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${FILE}  ${CMAKE_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME} )
    endforeach ()

    set_target_properties( ${PROJECT_NAME} PROPERTIES IMPORTED_LOCATION ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/ )

    execute_process( COMMAND  android update project --name ${ANDROID_APP_NAME} --target android-${ANDROID_TARGET_API_LEVEL} --path . )

    ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND  android update project --name ${ANDROID_APP_NAME} --target android-${ANDROID_TARGET_API_LEVEL} --path .
        COMMAND  ant release
    )

elseif( IOS )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${IOS_PLISTT}" 
        RESOURCE                 "${RESOURCES_LIST}"
        XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
    )

elseif( MACOS )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES
                            MACOSX_BUNDLE_INFO_PLIST "${MACOS_PLIST}" 
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                            RESOURCE "${RESOURCES_LIST}"
                          )

    ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND   
        install_name_tool -change @executable_path/../Frameworks/libfmodex.dylib  @executable_path/../Resources/libfmodex.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/Resources/libfmodevent.dylib  

        COMMAND   
        install_name_tool -change ./libfmodevent.dylib @executable_path/../Resources/libfmodevent.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}    

        COMMAND   
        install_name_tool -change ./libfmodex.dylib @executable_path/../Resources/libfmodex.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}   

        COMMAND 
        install_name_tool -change ./libIMagickHelper.dylib @executable_path/../Resources/libIMagickHelper.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}     

        COMMAND   
        install_name_tool -change ./libTextureConverter.dylib @executable_path/../Resources/libTextureConverter.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}   
    )

elseif ( MSVC )       
    if( "${EXECUTABLE_FLAG}" STREQUAL "WIN32")
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

list ( APPEND DAVA_FOLDERS ${DAVA_ENGINE_DIR} )
list ( APPEND DAVA_FOLDERS ${FILE_TREE_CHECK_FOLDERS} )
list ( APPEND DAVA_FOLDERS ${DAVA_THIRD_PARTY_LIBRARIES_PATH} )

file_tree_check( "${DAVA_FOLDERS}" )

target_link_libraries( ${PROJECT_NAME} ${LIBRARIES} ${DAVA_LIBRARY} )

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()


endmacro ()
