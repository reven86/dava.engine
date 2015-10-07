#
#set( APP_DATA       )

#set( IOS_PLISTT     )
#set( IOS_XIB        )
#
#set( MACOS_PLIST    )
#set( MACOS_ICO      )
#set( MACOS_DYLIB    )
#set( MACOS_XIB      )
#
#set( WIN32_RESOURCES )
#
#set( ANDROID_USE_STANDART_TEMLATE )
#set( ANDROID_PACKAGE              )
#set( ANDROID_APP_NAME             )
#set( ANDROID_ACTIVITY_APP_NAME    )
#set( ANDROID_JAVA_SRC             )
#set( ANDROID_JAVA_LIBS            )
#set( ANDROID_JAVA_RES             )
#set( ANDROID_JAVA_ASSET           )
#set( ANDROID_ICO                  )
#
#set( ADDED_SRC                  )
#set( LIBRARIES                  )
#set( LIBRARIES_RELEASE          )
#set( LIBRARIES_DEBUG            )
#set( ADDED_BINARY_DIR           )
#set( EXECUTABLE_FLAG            )
#set( FILE_TREE_CHECK_FOLDERS    )
#

# Only interpret ``if()`` arguments as variables or keywords when unquoted.
if(NOT (CMAKE_VERSION VERSION_LESS 3.1))
    cmake_policy(SET CMP0054 NEW)
endif()

macro( setup_main_executable )

include      ( PlatformSettings )

if( WIN32 )
    add_definitions ( -D_CRT_SECURE_NO_DEPRECATE )
endif()

if( MACOS_DATA )
    set( APP_DATA ${MACOS_DATA} )

elseif( WIN32_DATA )
    set( APP_DATA ${WIN32_DATA} )

elseif( IOS_DATA )
    set( APP_DATA ${IOS_DATA} )

elseif( ANDROID_DATA )
    set( APP_DATA ${ANDROID_DATA} )

endif()

if( ANDROID )
    if( NOT ANDROID_JAVA_SRC )
        list( APPEND ANDROID_JAVA_SRC  ${CMAKE_CURRENT_LIST_DIR}/android/src )
    endif()

    if( NOT ANDROID_JAVA_RES )
        set( ANDROID_JAVA_RES  ${CMAKE_CURRENT_LIST_DIR}/android/res )

    endif()

endif()

if( DAVA_TOOLS_FOUND )
    include_directories( ${DAVA_TOOLS_DIR} )
    list ( APPEND LIBRARIES ${DAVA_TOOLS_LIBRARY} )

endif()

if( DAVA_FOUND )
    include_directories   ( ${DAVA_INCLUDE_DIR} )
    include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

    list( APPEND ANDROID_JAVA_LIBS  ${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/jar )
    list( APPEND ANDROID_JAVA_SRC   ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/Java )

endif()

if( IOS )
    list( APPEND RESOURCES_LIST ${APP_DATA} )
    list( APPEND RESOURCES_LIST ${IOS_XIB} )
    list( APPEND RESOURCES_LIST ${IOS_PLIST} )
    list( APPEND RESOURCES_LIST ${IOS_ICO} )

elseif( MACOS )
    if( DAVA_FOUND )
        file ( GLOB DYLIB_FILES    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.dylib)
    endif()

    set_source_files_properties( ${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

    list ( APPEND DYLIB_FILES     "${DYLIB_FILES}" "${MACOS_DYLIB}" )

    list( APPEND RESOURCES_LIST  ${APP_DATA}  )
    list( APPEND RESOURCES_LIST  ${DYLIB_FILES} )
    list( APPEND RESOURCES_LIST  ${MACOS_XIB}   )
    list( APPEND RESOURCES_LIST  ${MACOS_PLIST} )
    list( APPEND RESOURCES_LIST  ${MACOS_ICO}   )

    list( APPEND LIBRARIES      ${DYLIB_FILES} )

elseif ( WINDOWS_UAP )

    if ( NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore" )
        message(FATAL_ERROR "This app supports Store / Phone only. Please check the target platform.")
    endif ()

    set ( WINDOWS_UAP_CONFIG_DIR "${DAVA_ROOT_DIR}/Sources/CMake/ConfigureFiles" )

    #loading config file
    if ( NOT WINDOWS_UAP_CONFIG_FILE )
        set ( WINDOWS_UAP_CONFIG_FILE "${WINDOWS_UAP_CONFIG_DIR}/UWPConfigTemplate.in" )
    endif ()
    configure_file( ${WINDOWS_UAP_CONFIG_FILE} ${CMAKE_CURRENT_BINARY_DIR}/UWPConfig.in )
    load_config ( ${CMAKE_CURRENT_BINARY_DIR}/UWPConfig.in )

    set ( APP_MANIFEST_NAME "Package.appxmanifest" )
    set ( APP_CERT_NAME "${PROJECT_NAME}_Key.pfx" )
    set ( SHORT_NAME ${PROJECT_NAME} )
    set_property ( GLOBAL PROPERTY USE_FOLDERS ON )

    #search assets
    file( GLOB ASSET_FILES "${WINDOWS_UAP_ASSETS_DIR}/*.png" )
    source_group ( "Content\\Assets" FILES ${ASSET_FILES} )

    #copy manifest
    configure_file ( ${WINDOWS_UAP_MANIFEST_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${APP_MANIFEST_NAME} @ONLY )

    #copy key file
    get_filename_component ( CERT_NAME ${WINDOWS_UAP_CERTIFICATE_FILE} NAME )
    file ( COPY ${WINDOWS_UAP_CERTIFICATE_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
    file ( RENAME ${CMAKE_CURRENT_BINARY_DIR}/${CERT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${APP_CERT_NAME} )

    #copy priconfig files
    file ( COPY "${WINDOWS_UAP_CONFIG_DIR}/UWPPriConfigDefault.xml" DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
    file ( COPY "${WINDOWS_UAP_CONFIG_DIR}/UWPPriConfigPackaging.xml" DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

    set(CONTENT_FILES ${CONTENT_FILES}
        ${CMAKE_CURRENT_BINARY_DIR}/${APP_MANIFEST_NAME} )

    set(RESOURCE_FILES ${CONTENT_FILES} ${DEBUG_CONTENT_FILES} ${RELEASE_CONTENT_FILES}
        ${ASSET_FILES} ${STRING_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${APP_CERT_NAME} )
    list( APPEND RESOURCES_LIST ${RESOURCE_FILES} )

	#add dll's to project and package
	file ( GLOB DAVA_DEBUG_DLL_LIST   "${DAVA_WIN_UAP_LIBRARIES_PATH_DEBUG}/*.dll" )
	file ( GLOB DAVA_RELEASE_DLL_LIST "${DAVA_WIN_UAP_LIBRARIES_PATH_RELEASE}/*.dll" )

	if ( DAVA_DEBUG_DLL_LIST )
	    source_group ("Binaries\\Debug"   FILES ${DAVA_DEBUG_DLL_LIST})
		set_property(SOURCE ${DAVA_DEBUG_DLL_LIST} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
	endif ()

	if ( DAVA_RELEASE_DLL_LIST )
	    source_group ("Binaries\\Release" FILES ${DAVA_RELEASE_DLL_LIST})
        set_property(SOURCE ${DAVA_RELEASE_DLL_LIST} PROPERTY
		    VS_DEPLOYMENT_CONTENT $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>)
	endif ()

    list( APPEND ADDED_SRC "${DAVA_DEBUG_DLL_LIST}"
                           "${DAVA_RELEASE_DLL_LIST}" )

	set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
	set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
	set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
	set_property(SOURCE ${STRING_FILES} PROPERTY VS_TOOL_OVERRIDE "PRIResource")
	set_property(SOURCE ${DEBUG_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
	set_property(SOURCE ${RELEASE_CONTENT_FILES} PROPERTY
		VS_DEPLOYMENT_CONTENT $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>)

	set ( UAP_DEPLOYMENT_CONTENT ${APP_DATA} )
	list ( APPEND UAP_DEPLOYMENT_CONTENT ${ADDITIONAL_CONTENT} )
	add_content_win_uap ( "${UAP_DEPLOYMENT_CONTENT}" )
	list( APPEND ADDED_SRC ${ADDED_CONTENT_SRC} )

elseif( WIN32 )
    list( APPEND RESOURCES_LIST  ${WIN32_RESOURCES} )
endif()

###

if( DAVA_FOUND )
    if( ANDROID )
        include_directories   ( ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )
        list( APPEND PATTERNS_CPP    ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/*.cpp )
        list( APPEND PATTERNS_H      ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/*.h   )

        list( APPEND PATTERNS_CPP    ${ANDROID_NDK}/sources/android/cpufeatures/*.c )
        list( APPEND PATTERNS_H      ${ANDROID_NDK}/sources/android/cpufeatures/*.h )

    endif()

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

    else()
        if( WIN32 )
            add_definitions        ( -D_UNICODE
                                     -DUNICODE )
            list( APPEND ADDED_SRC  ${DAVA_PLATFORM_SRC}/TemplateWin32/CorePlatformWin32.cpp
                                    ${DAVA_PLATFORM_SRC}/TemplateWin32/CorePlatformWin32.h  )

        endif()

    endif()

    file( GLOB_RECURSE CPP_FILES ${PATTERNS_CPP} )
    file( GLOB_RECURSE H_FILES   ${PATTERNS_H} )
    set ( PLATFORM_ADDED_SRC ${H_FILES} ${CPP_FILES} )

endif()

###

if( ANDROID )
    set( POSTFIX 0  )
    set( COUNTER 0 )
    set( SRC_LIST  )
    set( REMAINING_LIST  )

    foreach( ITEM ${PROJECT_SOURCE_FILES} )
        get_filename_component( ITEM_EXT ${ITEM} EXT )

        if( ${ITEM_EXT} STREQUAL ".cpp" )
            list( APPEND SRC_LIST  ${ITEM} )
            math( EXPR COUNTER "${COUNTER} + 1" )

            if( ${COUNTER} GREATER ${DAVA_ANDROID_MAX_LIB_SRC} )
                math( EXPR POSTFIX "${POSTFIX} + 1" )

                set( LIB_NAME "${PROJECT_NAME}_${POSTFIX}"  )
                add_library( ${LIB_NAME} STATIC ${SRC_LIST} )
                list( APPEND TARGET_LIBRARIES ${LIB_NAME} )

                set( COUNTER 0 )
                set( SRC_LIST )

            endif()

        else()
            list( APPEND REMAINING_LIST  ${ITEM} )

        endif()

    endforeach()

    if( ${COUNTER} GREATER 0 )
        math( EXPR POSTFIX "${POSTFIX} + 1" )

        set( LIB_NAME "${PROJECT_NAME}_${POSTFIX}"  )
        add_library( ${LIB_NAME} STATIC ${SRC_LIST} )
        list( APPEND TARGET_LIBRARIES ${LIB_NAME} )

        set( COUNTER 0 )
        set( SRC_LIST )

    endif()

    add_library( ${PROJECT_NAME} SHARED ${PLATFORM_ADDED_SRC} ${ADDED_SRC} ${REMAINING_LIST} )

else()
    if( NOT MAC_DISABLE_BUNDLE )
        set( BUNDLE_FLAG  MACOSX_BUNDLE )
    endif()

    add_executable( ${PROJECT_NAME} ${BUNDLE_FLAG} ${EXECUTABLE_FLAG}
        ${ADDED_SRC}
        ${PLATFORM_ADDED_SRC}
        ${PROJECT_SOURCE_FILES}
        ${RESOURCES_LIST}
    )

endif()

if (QT5_FOUND)
    link_with_qt5(${PROJECT_NAME})
endif()

if ( QT5_FOUND )
    if ( WIN32 )
        set ( QTCONF_DEPLOY_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/qt.conf" )
    elseif ( APPLE )
        set ( QTCONF_DEPLOY_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PROJECT_NAME}.app/Contents/Resources/qt.conf" )
    endif()

    if     ( TEAMCITY_DEPLOY AND WIN32 )
        set ( PLUGINS_PATH .)
    elseif ( TEAMCITY_DEPLOY AND APPLE )
        set ( PLUGINS_PATH PlugIns )
    else()
        set( PLUGINS_PATH  ${QT5_LIB_PATH}/../plugins )
        get_filename_component( PLUGINS_PATH ${PLUGINS_PATH} ABSOLUTE )
    endif()

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/QtConfTemplate.in
                    ${CMAKE_CURRENT_BINARY_DIR}/DavaConfigDebug.in  )
    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/QtConfTemplate.in
                    ${CMAKE_CURRENT_BINARY_DIR}/DavaConfigRelWithDebinfo.in  )
    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/QtConfTemplate.in
                    ${CMAKE_CURRENT_BINARY_DIR}/DavaConfigRelease.in  )

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy
       ${CMAKE_CURRENT_BINARY_DIR}/DavaConfig$(CONFIGURATION).in
       ${QTCONF_DEPLOY_PATH}
    )

endif()


if( ANDROID AND NOT ANDROID_CUSTOM_BUILD )
    set( LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_NDK_ABI_NAME}" CACHE PATH "Output directory for Android libs" )

    set( ANDROID_MIN_SDK_VERSION     ${ANDROID_NATIVE_API_LEVEL} )
    set( ANDROID_TARGET_SDK_VERSION  ${ANDROID_TARGET_API_LEVEL} )

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

    if( APP_DATA )
        get_filename_component( ASSETS_FOLDER ${APP_DATA} NAME )
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${APP_DATA} ${CMAKE_BINARY_DIR}/assets/${ASSETS_FOLDER} )
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

    if( NOT CMAKE_EXTRA_GENERATOR )
        add_custom_target( ant-configure ALL
            COMMAND  ${ANDROID_COMMAND} update project --name ${ANDROID_APP_NAME} --target android-${ANDROID_TARGET_API_LEVEL} --path ${CMAKE_CURRENT_BINARY_DIR} --subprojects
            COMMAND  ${ANT_COMMAND} release
        )

        add_dependencies( ant-configure ${PROJECT_NAME} )
    else()
        execute_process( COMMAND ${ANDROID_COMMAND} update project --name ${ANDROID_APP_NAME} --target android-${ANDROID_TARGET_API_LEVEL} --path ${CMAKE_CURRENT_BINARY_DIR} --subprojects )
    endif()


elseif( IOS )
    set_target_properties( ${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${IOS_PLISTT}"
        RESOURCE                 "${RESOURCES_LIST}"
        XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
    )

    foreach ( TARGET ${PROJECT_NAME} ${DAVA_LIBRARY}  )
        set_xcode_property( ${TARGET} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] YES )
        set_xcode_property( ${TARGET} ONLY_ACTIVE_ARCH YES )
    endforeach ()

    # Universal (iPad + iPhone)
    set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" )

elseif( MACOS )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES
                            MACOSX_BUNDLE_INFO_PLIST "${MACOS_PLIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                            RESOURCE "${RESOURCES_LIST}"
                          )

    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-dead_strip")

    if( DEPLOY )
        set( OUTPUT_DIR ${DEPLOY_DIR}/${PROJECT_NAME}.app/Contents )

    else()
        set( OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.app/Contents )
    endif()

    set( BINARY_DIR ${OUTPUT_DIR}/MacOS/${PROJECT_NAME} )

    if( DAVA_FOUND )
        set(LD_RUNPATHES "@executable_path @executable_path/../Resources @executable_path/../Frameworks")
        set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${LD_RUNPATHES}")
    endif()

elseif ( WIN32 )
    if( "${EXECUTABLE_FLAG}" STREQUAL "WIN32" )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY: /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib" )

    else()
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib" )

    endif()


    if( DEBUG_INFO )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /SUBSYSTEM:WINDOWS" )
    else()
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS" )
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

if( TARGET_FILE_TREE_FOUND )
    add_dependencies(  ${PROJECT_NAME} FILE_TREE_${PROJECT_NAME} )

endif()

if( DAVA_FOUND )
    list ( APPEND LIBRARIES ${DAVA_LIBRARY} )

endif()

if( ANDROID )
    set( LINK_WHOLE_ARCHIVE_FLAG -Wl,--whole-archive -Wl,--allow-multiple-definition )
    set( NO_LINK_WHOLE_ARCHIVE_FLAG -Wl,--no-whole-archive )

    foreach( LIB_1 ${TARGET_LIBRARIES} )
        foreach( LIB_2 ${TARGET_LIBRARIES} )
            if( ${LIB_1} STREQUAL ${LIB_2} )
            else()
                target_link_libraries( ${LIB_1} ${LINK_WHOLE_ARCHIVE_FLAG} ${LIB_2} ${NO_LINK_WHOLE_ARCHIVE_FLAG} ${LIBRARIES} )
            endif()
        endforeach()
    endforeach()

endif()

target_link_libraries( ${PROJECT_NAME} ${LINK_WHOLE_ARCHIVE_FLAG} ${TARGET_LIBRARIES} ${NO_LINK_WHOLE_ARCHIVE_FLAG} ${LIBRARIES} )

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()


###

if( DEPLOY )
   message( "DEPLOY ${PROJECT_NAME} to ${DEPLOY_DIR}")
   execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR} )

    if( WIN32 )
        if( APP_DATA )
            get_filename_component( DIR_NAME ${APP_DATA} NAME )

            ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
               COMMAND ${CMAKE_COMMAND} -E copy_directory ${APP_DATA}  ${DEPLOY_DIR}/${DIR_NAME}/
               COMMAND ${CMAKE_COMMAND} -E remove  ${DEPLOY_DIR}/${PROJECT_NAME}.ilk
            )

        endif()

        foreach ( ITEM fmodex.dll fmod_event.dll IMagickHelper.dll glew32.dll TextureConverter.dll )
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_TOOLS_BIN_DIR}/${ITEM}  ${DEPLOY_DIR} )
        endforeach ()

        set( OUTPUT_DIR "${DEPLOY_DIR}" )
        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
            set_target_properties ( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR} )
        endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

    elseif( APPLE )
        set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR  ${DEPLOY_DIR} )

        if( IOS )

            if( NOT IOS_SDK )
                set( IOS_SDK -sdk iphoneos  )
            endif()

            set( XCODERUN_PARAM ${IOS_SDK} PackageApplication -v ${DEPLOY_DIR}/${PROJECT_NAME}.app -o ${DEPLOY_DIR}/${PROJECT_NAME}.ipa )

            if( DEVELOPER_NAME )
                list( APPEND XCODERUN_PARAM  "--sign" "${DEVELOPER_NAME}" )
            endif()

            if( PROVISONING_PROFILE )
                list( APPEND XCODERUN_PARAM  "--embed" "${PROVISONING_PROFILE}" )
            endif()

            add_custom_target ( IOS_DEPLOY_${PROJECT_NAME} ALL COMMAND ${IOS_DEPLOY_CUSTOM_COMAND}
                                                               COMMAND /usr/bin/xcrun ${XCODERUN_PARAM} VERBATIM )


            add_dependencies(  IOS_DEPLOY_${PROJECT_NAME} ${PROJECT_NAME} )

        endif()

    endif()

    if( QT5_FOUND )
        qt_deploy( )

    endif()

endif()

endmacro ()

macro( DEPLOY_SCRIPT )

    if( DEPLOY )
        cmake_parse_arguments (ARG "" "" "PYTHON;COPY;COPY_WIN32;COPY_MACOS;COPY_DIR" ${ARGN})

        if( NOT COPY_DIR )
            set( COPY_DIR ${DEPLOY_DIR} )
        endif()

        execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${COPY_DIR} )
        execute_process( COMMAND python ${ARG_PYTHON} )

        if( ARG_COPY )
            list( APPEND COPY_LIST ${ARG_COPY} )
        endif()

        if( ARG_COPY_WIN32 AND WIN32 )
            list( APPEND COPY_LIST ${ARG_COPY_WIN32} )
        endif()

        if( ARG_COPY_MACOS AND MACOS )
            list( APPEND COPY_LIST ${ARG_COPY_MACOS} )
        endif()

        foreach ( ITEM ${COPY_LIST} )
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${ITEM} ${COPY_DIR} )
        endforeach ()

    endif()
endmacro ()

