
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
        list( APPEND ERASE_FILES     )

        set(CMAKE_C_FLAGS  ${CMAKE_C_FLAGS} "-x objective-c")
        set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} "-x objective-c++")
        set_source_files_properties( ${CMAKE_CURRENT_LIST_DIR}/ThreadPosix.cpp PROPERTIES COMPILE_FLAGS "-x Objective-C++" )  

    endif()

elseif( WIN32 ) 
    set( ADDED_CPP_PATTERNS TemplateWin32/*.cpp  )
    set( ADDED_H_PATTERNS   TemplateWin32/*.h    )
    list( APPEND ERASE_FILES  CorePlatformWin32.cpp CorePlatformWin32.h  )
 
endif()


define_source_files (
GLOB_CPP_PATTERNS  *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS    *.h   ${ADDED_H_PATTERNS}
GLOB_ERASE_FILES   ${ERASE_FILES}
PARENT_SCOPE)  

