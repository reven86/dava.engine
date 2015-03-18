include ( GlobalVariables )

macro ( qt_deploy )
    if ( NOT QT4_FOUND )
        return ()
    endif ()

    if( WIN32 )
        set( BINARY_ITEMS QtCore4.dll
                          QtGui4.dll
                          )

        foreach ( ITEM  ${BINARY_ITEMS} )
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${QT4_PATH_WIN}/bin/${ITEM}  ${DEPLOY_DIR} )

        endforeach ()

    elseif( MACOS )

        get_filename_component ( QT_PATH ${QT_MOC_EXECUTABLE} PATH ) 
        ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD 
            COMMAND ${QT_PATH}/macdeployqt ${DEPLOY_DIR}/${PROJECT_NAME}.app
        )

    endif()

endmacro ()

if ( QT4_FOUND )
    return ()
endif ()

if ( MSVC )
    set( CMAKE_PREFIX_PATH    "${QT4_PATH_WIN}")
    set( QT_MOC_EXECUTABLE    "${QT4_PATH_WIN}/bin/moc.exe" )
    set( QT_RCC_EXECUTABLE    "${QT4_PATH_WIN}/bin/rcc.exe" )
    set( QT_UIC_EXECUTABLE    "${QT4_PATH_WIN}/bin/uic.exe" )
    set( QT_BINARY_DIR        "${QT4_PATH_WIN}/bin"         )
    message ( "QT4_PATH_WIN      -"  ${QT4_PATH_WIN}      )
    message ( "QT_MOC_EXECUTABLE -"  ${QT_MOC_EXECUTABLE} )
    message ( "QT_RCC_EXECUTABLE -"  ${QT_RCC_EXECUTABLE} )
    message ( "QT_UIC_EXECUTABLE -"  ${QT_UIC_EXECUTABLE} )
endif()

set( TMP_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} )
set( CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules" )
find_package( Qt4 REQUIRED )
set( CMAKE_MODULE_PATH "${TMP_CMAKE_MODULE_PATH}" )

if( NOT QT4_FOUND )
    message( "Error !!!: Please set the correct path to QT4 in file DavaConfig.in"  )
    message( " " )
    exit()

endif()
