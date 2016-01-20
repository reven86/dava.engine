include ( GlobalVariables )
include ( CMake-common )

macro ( qt_deploy )
    if ( NOT QT5_FOUND )
        return ()
    endif ()

    set(DEPLOY_SCRIPT_PATH ${DAVA_SCRIPTS_FILES_PATH}/deployQt.py)

    if( WIN32 )
        get_qt5_deploy_list(BINARY_ITEMS)

        foreach(ITEM ${BINARY_ITEMS})
            string(TOLOWER ${ITEM} ITEM)
            if (EXISTS ${QT5_PATH_WIN}/bin/Qt5${ITEM}.dll)
                LIST(APPEND QT_ITEMS_LIST --${ITEM})
            endif()
        endforeach()

        if (NOT QML_SCAN_DIR)
            set(QML_SCAN_DIR " ")
        endif()

        set(DEPLOY_CURRENT_FOLDER ${QT5_PATH_WIN}/bin/)
        set(DEPLOT_COMMAND "windeployqt.exe")
        set(DEPLOT_COMMAND "${DEPLOT_COMMAND} $<$<CONFIG:Debug>:--debug> $<$<NOT:$<CONFIG:Debug>>:--release>")
        set(DEPLOT_COMMAND "${DEPLOT_COMMAND} --dir ${DEPLOY_DIR}")
        set(DEPLOT_COMMAND "${DEPLOT_COMMAND} --qmldir ${QML_SCAN_DIR} $<TARGET_FILE:${PROJECT_NAME}>")
        set(DEPLOT_COMMAND "${DEPLOT_COMMAND} ${QT_ITEMS_LIST}")

        

    elseif( MACOS )

        if (QML_SCAN_DIR)
            set(QML_SCAN_FLAG "-qmldir=${QML_SCAN_DIR}")
        endif()

        set(DEPLOY_CURRENT_FOLDER ${DEPLOY_DIR})
        set(DEPLOT_COMMAND "${QT5_PATH_MAC}/bin/macdeployqt ${PROJECT_NAME}.app -always-overwrite ${QML_SCAN_FLAG}")

    endif()

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
            COMMAND "python"
                    ${DEPLOY_SCRIPT_PATH}
                    ${DEPLOY_CURRENT_FOLDER}
                    ${DEPLOT_COMMAND}
        )

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

list( APPEND QT5_FIND_COMPONENTS ${QT5_FIND_COMPONENTS} Core Gui Widgets Concurrent Qml Quick Network)
list( REMOVE_DUPLICATES QT5_FIND_COMPONENTS)

resolve_qt_pathes()

foreach(COMPONENT ${QT5_FIND_COMPONENTS})
    if (NOT Qt5${COMPONENT}_FOUND)
        find_package("Qt5${COMPONENT}")
    endif()

    ASSERT(Qt5${COMPONENT}_FOUND "Can't find Qt5 component : ${COMPONENT}")
    LIST(APPEND DEPLOY_LIST "Qt5${COMPONENT}")
    LIST(APPEND LINKAGE_LIST "Qt5::${COMPONENT}")
endforeach()

append_qt5_deploy(DEPLOY_LIST)
set_linkage_qt5_modules(LINKAGE_LIST)
set ( DAVA_EXTRA_ENVIRONMENT QT_QPA_PLATFORM_PLUGIN_PATH=$ENV{QT_QPA_PLATFORM_PLUGIN_PATH} )

set(QT5_FOUND 1)

if( NOT QT5_FOUND )
    message( FATAL_ERROR "Please set the correct path to QT5 in file DavaConfig.in"  )
endif()
