
if( ANDROID )
    set( ADDED_CPP_PATTERNS TemplateAndroid/*.cpp  TemplateAndroid/ExternC/*.cpp ) 	
    set( ADDED_H_PATTERNS   TemplateAndroid/*.h    TemplateAndroid/ExternC/*.h   )
    set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR} "${CMAKE_CURRENT_LIST_DIR}/TemplateAndroid" PARENT_SCOPE)
elseif( APPLE ) 
    if( IOS )
    	set( ADDED_CPP_PATTERNS TemplateiOS/*.cpp TemplateiOS/*.mm )
    	set( ADDED_H_PATTERNS TemplateiOS/*.h )

        set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR} "${CMAKE_CURRENT_LIST_DIR}/TemplateiOS/" PARENT_SCOPE)
        set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR} "${CMAKE_CURRENT_LIST_DIR}/" PARENT_SCOPE)
    else()
        set( ADDED_CPP_PATTERNS *.mm *.m TemplateMacOS/*.cpp TemplateMacOS/*.mm TemplateMacOS/*.m Qt/*.cpp Qt/MacOS/*.cpp Qt/MacOS/*.mm )
    	set( ADDED_H_PATTERNS TemplateMacOS/*.h Qt/*.h   Qt/MacOS/*.h )
        set( ERASE_FILES  MainWindowController.h MainWindowController.mm OpenGLView.h OpenGLView.mm )
    endif()
else()
    set( ADDED_CPP_PATTERNS TemplateWin32/*.cpp Qt/*.cpp Qt/Win32/*.cpp )
    set( ADDED_H_PATTERNS   TemplateWin32/*.h   Qt/*.h   Qt/Win32/*.h   )

endif()

define_source_files (
GLOB_CPP_PATTERNS  *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS    *.h   ${ADDED_H_PATTERNS}
GLOB_ERASE_FILES   ${ERASE_FILES}
PARENT_SCOPE)  

