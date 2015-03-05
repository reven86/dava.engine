include ( GlobalVariables )

if ( QT5_FOUND )
    return ()
endif ()

if( WIN32 )
    set ( QT_CORE_LIB Qt5Core.lib )

elseif( MACOS )
    set ( QT_CORE_LIB QtCore.la )

endif()

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

endif()

if( NOT QT5_FOUND )
    message( "Error !!!: Please set the correct path to QT5 in file DavaConfig.in"  )
    message( " " )
    exit()

endif()
