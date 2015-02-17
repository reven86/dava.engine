#
#set( MACOS_PLIST          )
#set( MACOS_ICO            )
#set( MACOS_DATA           )
#set( MACOS_DYLIB          )
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
    set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )

    set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )

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

add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${EXECUTABLE_FLAG}
    ${ADDED_SRC} 
    ${PROJECT_SOURCE_FILES} 
    ${RESOURCES_LIST}
)

if( APPLE )

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
        install_name_tool -change ./libTextureConverter.dylib @executable_path/../Frameworks/libTextureConverter.dylib ${CMAKE_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}   
    )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES
                            MACOSX_BUNDLE_INFO_PLIST ${MACOS_PLIST} 
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                            RESOURCE "${MACOS_ICO}"
                          )

elseif ( MSVC )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"" )    
    
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
