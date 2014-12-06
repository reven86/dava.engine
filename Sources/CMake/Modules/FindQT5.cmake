include ( GlobalVariables )

if( WIN32 )
    set ( QT5_PATH ${QT5_PATH_WIN32} CACHE PATH "Path to Qt5")
    set ( QT_LIB Qt5Core.dll )

elseif( MACOS )
    set ( QT5_PATH ${QT5_PATH_MAC} CACHE PATH "Path to Qt5")
    set ( QT_LIB Qt5Core )

endif()

set ( QT_FOUND 0 )

find_path( EXISTS_QT 
  NAMES 
    ${QT_LIB}
  HINTS 
    ENV QT5_HOME
  PATHS 
    ${QT5_PATH}
  PATH_SUFFIXES 
    bin
)

if( EXISTS_QT )
    set ( QT5_MODULE_PATH ${QT5_PATH}/lib/cmake)
    set ( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_MODULE_PATH})

    find_package ( Qt5Core )

    if( Qt5Core_FOUND  )
        find_package ( Qt5Concurrent )	
        find_package ( Qt5Gui )
        find_package ( Qt5Widgets )

        if( Qt5Concurrent_FOUND AND
            Qt5Gui_FOUND        AND
            Qt5Widgets_FOUND   )
    
            set ( QT_FOUND     1 )
            set ( QT_LIBRARIES Qt5::Core 
                               Qt5::Gui 
                               Qt5::Widgets 
                               Qt5::Concurrent )

        endif()

    endif()

endif()

if( NOT QT_FOUND )
    message( "Error !!!: Please set the correct path to QT5 in file DavaConfig.in"  )
    message( " " )
    exit()

endif()
