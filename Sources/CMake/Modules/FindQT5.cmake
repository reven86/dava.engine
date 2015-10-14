include ( GlobalVariables )
include ( CMake-common )

macro ( qt_deploy )
    if ( NOT QT5_FOUND )
        return ()
    endif ()

    if( WIN32 )
        get_qt5_deploy_list(BINARY_ITEMS)

        foreach(ITEM ${BINARY_ITEMS})
            string(TOLOWER ${ITEM} ITEM)
            if (EXISTS ${QT5_PATH_WIN}/bin/Qt5${ITEM}.dll)
                LIST(APPEND QT_ITEMS_LIST --${ITEM})
            endif()
        endforeach()

        ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${DAVA_SCRIPTS_FILES_PATH}/deployqt.bat"
            "${QT5_PATH_WIN}/bin/"
            $<$<CONFIG:Debug>:--debug> $<$<NOT:$<CONFIG:Debug>>:--release>
            --dir  "${DEPLOY_DIR}/"
            --qmldir "${QML_SCAN_DIR}" "$<TARGET_FILE:${PROJECT_NAME}>"
            ${QT_ITEMS_LIST}
        )

    elseif( MACOS )
        if (BW_BUNDLE_NAME)
            set(BUNDLE_NAME ${BW_BUNDLE_NAME})
        else()
            set(BUNDLE_NAME ${PROJECT_NAME})
        endif()

        ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
            COMMAND ${QT5_PATH_MAC}/bin/macdeployqt
                    ${DEPLOY_DIR}/${BUNDLE_NAME}.app
                    -always-overwrite
                    -qmldir="${QML_SCAN_DIR}"
        )

    endif()

endmacro ()

macro(resolve_qt_pathes)
    if ( NOT QT5_LIB_PATH)

        if( WIN32 )
            set ( QT_CORE_LIB Qt5Core.lib )
        elseif( MACOS )
            set ( QT_CORE_LIB QtCore.la )
        endif()

        find_path( QT5_LIB_PATH NAMES ${QT_CORE_LIB}
                          PATHS ${QT5_PATH_MAC} ${QT5_PATH_WIN}
                          PATH_SUFFIXES lib)

        ASSERT(QT5_LIB_PATH "Please set the correct path to QT5 in file DavaConfig.in")

        set ( QT5_MODULE_PATH ${QT5_LIB_PATH}/cmake)
        set ( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_MODULE_PATH})

        message ( "QT5_LIB_PATH - " ${QT5_LIB_PATH} )

    endif()
endmacro()

#################################################################

# Find includes in corresponding build directories
set ( CMAKE_INCLUDE_CURRENT_DIR ON )
# Instruct CMake to run moc automatically when needed.
set ( CMAKE_AUTOMOC ON )

list( APPEND QT5_FIND_COMPONENTS ${QT5_FIND_COMPONENTS} Core Gui Widgets Concurrent)
list( REMOVE_DUPLICATES QT5_FIND_COMPONENTS)

resolve_qt_pathes()

foreach(COMPONENT ${QT5_FIND_COMPONENTS})
    if (NOT Qt5${COMPONENT}_FOUND)
        find_package("Qt5${COMPONENT}")
    endif()

    ASSERT(Qt5${COMPONENT}_FOUND "Can't find Qt5 component : ${COMPONENT}")
    LIST(APPEND LINKAGE_LIST "Qt5::${COMPONENT}")
endforeach()

append_qt5_deploy(QT5_FIND_COMPONENTS)
set_linkage_qt5_modules(LINKAGE_LIST)
set ( DAVA_EXTRA_ENVIRONMENT QT_QPA_PLATFORM_PLUGIN_PATH=$ENV{QT_QPA_PLATFORM_PLUGIN_PATH} )

set(QT5_FOUND 1)

if( NOT QT5_FOUND )
    message( FATAL_ERROR "Please set the correct path to QT5 in file DavaConfig.in"  )
endif()
