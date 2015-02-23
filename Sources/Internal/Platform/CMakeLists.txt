
if( QT_FOUND )
    set( QT_PREFIX "Qt4")	

elseif( QT5_FOUND )
    set( QT_PREFIX "Qt5")	

endif()

if( ANDROID )
#    set( ADDED_CPP_PATTERNS TemplateAndroid/*.cpp  TemplateAndroid/ExternC/*.cpp ) 	
#    set( ADDED_H_PATTERNS   TemplateAndroid/*.h    TemplateAndroid/ExternC/*.h   )
    set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR} "${CMAKE_CURRENT_LIST_DIR}/TemplateAndroid" PARENT_SCOPE)

elseif( APPLE ) 

    if( IOS )

    	set( ADDED_CPP_PATTERNS *.mm *.m TemplateiOS/*.cpp TemplateiOS/*.mm )
    	set( ADDED_H_PATTERNS TemplateiOS/*.h )

        set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR}
            "${CMAKE_CURRENT_LIST_DIR}/"
            "${CMAKE_CURRENT_LIST_DIR}/TemplateiOS/" PARENT_SCOPE )

    else()

        set( ADDED_CPP_PATTERNS *.mm *.m TemplateMacOS/*.cpp TemplateMacOS/*.mm TemplateMacOS/*.m  )
    	set( ADDED_H_PATTERNS TemplateMacOS/*.h  )

        if( QT_PREFIX )
            list ( APPEND ADDED_CPP_PATTERNS ${QT_PREFIX}/*.cpp ${QT_PREFIX}/MacOS/*.cpp ${QT_PREFIX}/MacOS/*.mm )
            list ( APPEND ADDED_H_PATTERNS   ${QT_PREFIX}/*.h   ${QT_PREFIX}/MacOS/*.h   )        
            list ( APPEND ERASE_FILES    NPAPICorePlatformMacOS.cpp NPAPICorePlatformMacOS.h ) #NPAPIPluginMacOS.h NPAPIPluginMacOS.mm
            set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR} ${CMAKE_CURRENT_LIST_DIR}/${QT_PREFIX}  ${CMAKE_CURRENT_LIST_DIR}/${QT_PREFIX}/MacOS PARENT_SCOPE)
        else()
            list ( APPEND ERASE_FILES    NPAPICorePlatformMacOS.cpp NPAPICorePlatformMacOS.h ) #NPAPIPluginMacOS.h NPAPIPluginMacOS.mm
        endif()

        set(CMAKE_C_FLAGS  ${CMAKE_C_FLAGS} "-x objective-c")
        set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} "-x objective-c++")
        set_source_files_properties( ${CMAKE_CURRENT_LIST_DIR}/ThreadPosix.cpp PROPERTIES COMPILE_FLAGS "-x Objective-C++" )  

    endif()

elseif( WIN32 ) 

    set( ADDED_CPP_PATTERNS TemplateWin32/*.cpp  )
    set( ADDED_H_PATTERNS   TemplateWin32/*.h    )

    if( QT_PREFIX )
        list ( APPEND ADDED_CPP_PATTERNS ${QT_PREFIX}/*.cpp ${QT_PREFIX}/Win32/*.cpp )
        list ( APPEND ADDED_H_PATTERNS   ${QT_PREFIX}/*.h   ${QT_PREFIX}/Win32/*.h   )        
        list ( APPEND ERASE_FILES  CorePlatformWin32.cpp CorePlatformWin32.h  )
        set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR} ${CMAKE_CURRENT_LIST_DIR}/${QT_PREFIX}  ${CMAKE_CURRENT_LIST_DIR}/${QT_PREFIX}/Win32 PARENT_SCOPE)
    else()
        set( ERASE_FILES  CorePlatformWin32Qt.cpp CorePlatformWin32Qt.h  )
    endif()

endif()

define_source_files (
GLOB_CPP_PATTERNS  *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS    *.h   ${ADDED_H_PATTERNS}
GLOB_ERASE_FILES   ${ERASE_FILES}
PARENT_SCOPE)  

