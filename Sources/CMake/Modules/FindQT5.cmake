include ( GlobalVariables )
include ( CMake-common )

if (NOT QT_VERSION)
    set(QT_VERSION "QT")
endif()

if( WIN32 AND NOT X64_MODE )
    message( FATAL_ERROR "We don't support x86 platform on windows"  )
endif ()

set(QT_ACTUAL_PATH ${${QT_VERSION}_PATH})

macro ( qt_deploy )
    if ( NOT QT5_FOUND )
        return ()
    endif ()
    set(DEPLOY_SCRIPT_PATH ${DAVA_SCRIPTS_FILES_PATH}/deployQt.py)
    set(DEPLOY_ROOT_FOLDER ${DEPLOY_DIR})

    if( WIN32 )
        get_qt5_deploy_list(BINARY_ITEMS)

        if (QML_SCAN_DIR)
            set(QML_SCAN_FLAG "--qmldir ${QML_SCAN_DIR}")
        endif()

        set(DEPLOY_PLATFORM "WIN")
        set(DEPLOY_QT_FOLDER ${QT_ACTUAL_PATH})
        set(DEPLOY_ARGUMENTS "$<$<CONFIG:Debug>:--debug> $<$<NOT:$<CONFIG:Debug>>:--release>")
        set(DEPLOY_ARGUMENTS "${DEPLOY_ARGUMENTS} --dir ${DEPLOY_DIR}")
        set(DEPLOY_ARGUMENTS "${DEPLOY_ARGUMENTS} ${QML_SCAN_FLAG}")
        foreach(ITEM ${BINARY_ITEMS})
            string(TOLOWER ${ITEM} ITEM)
            if (EXISTS ${QT_ACTUAL_PATH}/bin/Qt5${ITEM}.dll)
                set(DEPLOY_ARGUMENTS "${DEPLOY_ARGUMENTS} --${ITEM}")
            endif()
        endforeach()

    elseif( MACOS )

        if (QML_SCAN_DIR)
            set(QML_SCAN_FLAG "-qmldir=${QML_SCAN_DIR}")
        endif()

        set(DEPLOY_PLATFORM "MAC")
        set(DEPLOY_QT_FOLDER ${QT_ACTUAL_PATH})

        set(DEPLOY_ARGUMENTS "${PROJECT_NAME}.app -always-overwrite ${QML_SCAN_FLAG}")

    endif()
    
    if( QT_POST_DEPLOY AND NOT MACOS )
        ADD_CUSTOM_TARGET ( QT_DEPLOY ALL
                COMMAND "python"
                        ${DEPLOY_SCRIPT_PATH}
                        "-p" "${DEPLOY_PLATFORM}"
                        "-q" "${DEPLOY_QT_FOLDER}"
                        "-d" "${DEPLOY_ROOT_FOLDER}"
                        "-a" "${DEPLOY_ARGUMENTS}"
                        "-t" "${TARGETS_LIST}"
            )
            
        foreach( ITEM ${TARGETS_LIST} )
            add_dependencies( QT_DEPLOY ${ITEM} )           
        endforeach()
    else()
        ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
                COMMAND "python"
                        ${DEPLOY_SCRIPT_PATH}
                        "-p" "${DEPLOY_PLATFORM}"
                        "-q" "${DEPLOY_QT_FOLDER}"
                        "-d" "${DEPLOY_ROOT_FOLDER}"
                        "-a" "${DEPLOY_ARGUMENTS}"
                        "-n" "${PROJECT_NAME}"
            )    
    endif()

endmacro()

#################################################################

# Find includes in corresponding build directories
set ( CMAKE_INCLUDE_CURRENT_DIR ON )
# Instruct CMake to run moc automatically when needed.
set ( CMAKE_AUTOMOC ON )

list( APPEND QT5_FIND_COMPONENTS ${QT5_FIND_COMPONENTS} Core Gui Widgets Concurrent Qml Quick QuickWidgets Network Test)
list( REMOVE_DUPLICATES QT5_FIND_COMPONENTS)

set ( QT_CMAKE_RULES "${QT_ACTUAL_PATH}/lib/cmake")

if (NOT EXISTS ${QT_CMAKE_RULES})
   message( FATAL_ERROR "Please set the correct path to QT5 in file DavaConfig.in"  ) 
endif()

set ( QT5_LIB_PATH "${QT_ACTUAL_PATH}/lib")
set ( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${QT_ACTUAL_PATH}/lib/cmake")

foreach(COMPONENT ${QT5_FIND_COMPONENTS})
    if (NOT Qt5${COMPONENT}_FOUND)
        find_package("Qt5${COMPONENT}")
    endif()
    include_directories( "${Qt5${COMPONENT}_INCLUDE_DIRS}" )

    ASSERT(Qt5${COMPONENT}_FOUND "Can't find Qt5 component : ${COMPONENT}")
    LIST(APPEND DEPLOY_LIST "${COMPONENT}")
    LIST(APPEND LINKAGE_LIST "Qt5::${COMPONENT}")
endforeach()

append_qt5_deploy(DEPLOY_LIST)
set_linkage_qt5_modules(LINKAGE_LIST)
set ( DAVA_EXTRA_ENVIRONMENT QT_QPA_PLATFORM_PLUGIN_PATH=$ENV{QT_QPA_PLATFORM_PLUGIN_PATH} )

set(QT5_FOUND 1)

set_property( GLOBAL PROPERTY QT5_FOUND 1 )

if( NOT QT5_FOUND )
    message( FATAL_ERROR "Please set the correct path to QT5 in file DavaConfig.in"  )
endif()
