# Only interpret ``if()`` arguments as variables or keywords when unquoted.
if(NOT (CMAKE_VERSION VERSION_LESS 3.1))
    cmake_policy(SET CMP0054 NEW)
endif()

macro( setup_main_dynlib )

include      ( PlatformSettings )

if( WIN32 )
    add_definitions ( -D_CRT_SECURE_NO_DEPRECATE )
endif()

if( MACOS_DATA )
    set( APP_DATA ${MACOS_DATA} )
elseif( WIN32_DATA )
    set( APP_DATA ${WIN32_DATA} )
endif()

if( WIN32 )
    list( APPEND RESOURCES_LIST  ${WIN32_RESOURCES} )
endif()

###

if (MACOS)
    if ( DAVA_FOUND )
        file (GLOB DYLIB_FILES ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.dylib)
    endif()

    set_source_files_properties(${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND DYLIB_FILES "${DYLIB_FILES}" "${MACOS_DYLIB}")

    list( APPEND LIBRARIES      ${DYLIB_FILES} )

elseif ( WIN32 )
    list(APPEND RESOURCES_LIST ${WIN32_RESOURCES})
endif()

if( DAVA_FOUND )
    include_directories   ( ${DAVA_INCLUDE_DIR} )
    include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

   if( QT5_FOUND )
        if( WIN32 )
            set ( PLATFORM_INCLUDES_DIR ${DAVA_PLATFORM_SRC}/Qt5 ${DAVA_PLATFORM_SRC}/Qt5/Win32 )
            list( APPEND PATTERNS_CPP   ${DAVA_PLATFORM_SRC}/Qt5/*.cpp ${DAVA_PLATFORM_SRC}/Qt5/Win32/*.cpp )
            list( APPEND PATTERNS_H     ${DAVA_PLATFORM_SRC}/Qt5/*.h   ${DAVA_PLATFORM_SRC}/Qt5/Win32/*.h   )
        elseif( MACOS )
            set ( PLATFORM_INCLUDES_DIR  ${DAVA_PLATFORM_SRC}/Qt5  ${DAVA_PLATFORM_SRC}/Qt5/MacOS )
            list( APPEND PATTERNS_CPP    ${DAVA_PLATFORM_SRC}/Qt5/*.cpp ${DAVA_PLATFORM_SRC}/Qt5/MacOS/*.cpp ${DAVA_PLATFORM_SRC}/Qt5/MacOS/*.mm )
            list( APPEND PATTERNS_H      ${DAVA_PLATFORM_SRC}/Qt5/*.h   ${DAVA_PLATFORM_SRC}/Qt5/MacOS/*.h   )
        endif()

        include_directories( ${PLATFORM_INCLUDES_DIR} )
    endif()

    file( GLOB_RECURSE CPP_FILES ${PATTERNS_CPP} )
    file( GLOB_RECURSE H_FILES   ${PATTERNS_H} )
    set ( PLATFORM_ADDED_SRC ${H_FILES} ${CPP_FILES} )
endif()

###

add_library( ${PROJECT_NAME} SHARED
        ${ADDED_SRC}
        ${PLATFORM_ADDED_SRC}
        ${PROJECT_SOURCE_FILES}
        ${RESOURCES_LIST}
)

if( TARGET_FILE_TREE_FOUND )
    add_dependencies(  ${PROJECT_NAME} FILE_TREE_${PROJECT_NAME} )
endif()

if ( QT5_FOUND )
    if ( WIN32 )
        set ( QTCONF_DEPLOY_PATH "${TOOL_OUTPUT_DIR}/qt.conf" )
    elseif ( APPLE )
        set ( QTCONF_DEPLOY_PATH "${TOOL_OUTPUT_DIR}/${BW_BUNDLE_NAME}.app/Contents/Resources/qt.conf" )
    endif()

    if     ( TEAMCITY_DEPLOY AND WIN32 )
        set ( PLUGINS_PATH .)
        set ( QML_IMPORT_PATH .)
        set ( QML2_IMPORT_PATH .)
    elseif ( TEAMCITY_DEPLOY AND MACOS )
        set ( PLUGINS_PATH PlugIns )
        set ( QML_IMPORT_PATH Resources/qml)
        set ( QML2_IMPORT_PATH Resources/qml)
    else()
        get_filename_component (ABS_QT_PATH "${QT5_LIB_PATH}/../" ABSOLUTE)
        set ( PLUGINS_PATH  ${ABS_QT_PATH}/plugins )
        set ( QML_IMPORT_PATH ${ABS_QT_PATH}/qml)
        set ( QML2_IMPORT_PATH ${ABS_QT_PATH}/qml)
    endif()

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/QtConfTemplate.in
                    ${CMAKE_CURRENT_BINARY_DIR}/DavaConfig.in  )

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       ${CMAKE_CURRENT_BINARY_DIR}/DavaConfig.in
       ${QTCONF_DEPLOY_PATH}
    )

endif()

if ( WIN32 )
    if( "${EXECUTABLE_FLAG}" STREQUAL "WIN32" )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY: /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib" )
    else()
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib" )
    endif()

    list( APPEND DAVA_BINARY_WIN32_DIR "${ADDED_BINARY_DIR}" )

    if ( WINDOWS_UAP )
        set ( DAVA_VCPROJ_USER_TEMPLATE "DavaWinUAPVcxprojUserTemplate.in" )
    else ()
        set ( DAVA_VCPROJ_USER_TEMPLATE "DavaVcxprojUserTemplate.in" )
    endif ()

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/${DAVA_VCPROJ_USER_TEMPLATE}
                    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user @ONLY )

    if( OUTPUT_TO_BUILD_DIR )
        set( OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR} )
        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
            set_target_properties ( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${OUTPUT_DIR} )
        endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
     endif()

    if ( WINDOWS_UAP )
        set_property(TARGET ${PROJECT_NAME} PROPERTY VS_WINRT_COMPONENT TRUE)
    endif()

endif()

list ( APPEND DAVA_FOLDERS ${DAVA_ENGINE_DIR} )
list ( APPEND DAVA_FOLDERS ${FILE_TREE_CHECK_FOLDERS} )
list ( APPEND DAVA_FOLDERS ${DAVA_THIRD_PARTY_LIBRARIES_PATH} )

file_tree_check( "${DAVA_FOLDERS}" )

if( DAVA_FOUND )
    list ( APPEND LIBRARIES ${DAVA_LIBRARY} )
    set(LD_RUNPATHES "@executable_path @executable_path/../Resources @executable_path/../Frameworks")
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${LD_RUNPATHES}")
endif()

if( DAVA_TOOLS_FOUND )
    list ( APPEND LIBRARIES ${DAVA_TOOLS_LIBRARY} )
endif()

target_link_libraries( ${PROJECT_NAME} ${LINK_WHOLE_ARCHIVE_FLAG} ${TARGET_LIBRARIES} ${NO_LINK_WHOLE_ARCHIVE_FLAG} ${LIBRARIES} )

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()

if (QT5_FOUND)
    link_with_qt5(${PROJECT_NAME})
endif()

if ( WIN32 )
        set(TARGET_RESOURCE_DIR ${TOOL_OUTPUT_DIR})
    elseif( MACOS )
        set(TARGET_RESOURCE_DIR ${TOOL_OUTPUT_DIR}/${BW_BUNDLE_NAME}.app/Contents/Resources)
    endif()

if( APP_DATA )
    get_filename_component( DIR_NAME ${APP_DATA} NAME )

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E copy_directory ${APP_DATA}  ${TARGET_RESOURCE_DIR}/${DIR_NAME}/
    )
endif()

if (WIN32)
    list(APPEND DYNAMIC_LIB_LIST fmodex.dll fmod_event.dll IMagickHelper.dll glew32.dll TextureConverter.dll )
    set( DYNAMIC_LIBS_PATH ${DAVA_TOOLS_BIN_DIR})
elseif (MACOS)
    list(APPEND DYNAMIC_LIB_LIST libfmodex.dylib libfmodevent.dylib libTextureConverter.dylib ${MACOS_DYLIB} )
    set( DYNAMIC_LIBS_PATH ${DAVA_THIRD_PARTY_LIBRARIES_PATH} )
endif()

foreach ( ITEM ${DYNAMIC_LIB_LIST})
    if (EXISTS ${ITEM})
        ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${ITEM}  ${TARGET_RESOURCE_DIR}/ )
    else()
        ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${DYNAMIC_LIBS_PATH}/${ITEM}  ${TARGET_RESOURCE_DIR}/ )
    endif()
endforeach()


if (DEPLOY AND DEPLOY_DIR)
    if (WIN32)
        qt_deploy()
    elseif(MACOS)
        add_custom_target ( QT_DEPLOY_${PROJECT_NAME} ALL
            COMMAND ${QT5_PATH_MAC}/bin/macdeployqt
                    ${DEPLOY_DIR}/${BW_BUNDLE_NAME}.app
                    -always-overwrite
                    -qmldir="${QML_SCAN_DIR}"
        )

        get_deploy_dependencies(DEPENDENCIES_LIST)
        foreach(DEPENDENCY ${DEPENDENCIES_LIST})
            message("Add dependency : " ${DEPENDENCY})
            add_dependencies( QT_DEPLOY_${PROJECT_NAME} ${DEPENDENCY} )
        endforeach()
    endif()
endif()

endmacro ()
