include ( GlobalVariables )

macro ( qt_deploy )
    if ( NOT QT5_FOUND )
        return ()
    endif ()

    if( WIN32 )
        set( BINARY_ITEMS Qt5Core.dll
                          Qt5Gui.dll
                          Qt5Widgets.dll
                          )

        foreach ( ITEM  ${BINARY_ITEMS} )
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${QT5_PATH_WIN}/bin/${ITEM}  ${DEPLOY_DIR} )

        endforeach ()

        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR}/platforms )
        execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${QT5_PATH_WIN}/plugins/platforms/qwindows.dll
                                                          ${DEPLOY_DIR}/platforms )

        file ( GLOB FILE_LIST ${QT5_PATH_WIN}/bin/icu*.dll )
        foreach ( ITEM  ${FILE_LIST} )
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${ITEM}  ${DEPLOY_DIR} )

        endforeach ()

    elseif( MACOS )

        ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
            COMMAND ${QT5_PATH_MAC}/bin/macdeployqt ${DEPLOY_DIR}/${PROJECT_NAME}.app
        )

    endif()

endmacro ()


if ( QT5_FOUND )
    return ()
endif ()

if( WIN32 )
    set ( QT_CORE_LIB Qt5Core.lib )
elseif( MACOS )
    set ( QT_CORE_LIB QtCore.la )
endif()

# Find includes in corresponding build directories
set ( CMAKE_INCLUDE_CURRENT_DIR ON )

# Instruct CMake to run moc automatically when needed.
set ( CMAKE_AUTOMOC ON )
set(AUTOMOC_MOC_OPTIONS PROPERTIES FOLDER CMakeAutomocTargets)

set ( QT5_FOUND 0 )

find_path( QT5_LIB_PATH
  NAMES
    ${QT_CORE_LIB}
  PATHS
    ${QT5_PATH_MAC}
    ${QT5_PATH_WIN}
  PATH_SUFFIXES
    lib
)


if( QT5_LIB_PATH )
    set ( QT5_MODULE_PATH ${QT5_LIB_PATH}/cmake)
    set ( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_MODULE_PATH})

    message ( "QT5_LIB_PATH - " ${QT5_LIB_PATH} )

    find_package ( Qt5Core )

    if( Qt5Core_FOUND  )
        find_package ( Qt5Concurrent )
        find_package ( Qt5Gui )
        find_package ( Qt5Widgets )

        if( Qt5Concurrent_FOUND AND
            Qt5Gui_FOUND        AND
            Qt5Widgets_FOUND   )

            set ( QT5_FOUND    1 )
            set ( QT_LIBRARIES Qt5::Core
                               Qt5::Gui
                               Qt5::Widgets
                               Qt5::Concurrent )

        endif()

    endif()

    set ( DAVA_EXTRA_ENVIRONMENT QT_QPA_PLATFORM_PLUGIN_PATH=$ENV{QT_QPA_PLATFORM_PLUGIN_PATH} )

endif()


if( NOT QT5_FOUND )
    message( FATAL_ERROR "Please set the correct path to QT5 in file DavaConfig.in"  )
endif()
