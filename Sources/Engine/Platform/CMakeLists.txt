
if(ANDROID)
    set( ADDED_CPP_PATTERNS TemplateAndroid/*.cpp  TemplateAndroid/ExternC/*.cpp ) 	
    set( ADDED_H_PATTERNS   TemplateAndroid/*.h    TemplateAndroid/ExternC/*.h   )
    set(INCLUDES_DIR  ${INCLUDES_DIR} "${CMAKE_CURRENT_LIST_DIR}/TemplateAndroid" PARENT_SCOPE)
elseif(APPLE) 
	if(IOS)
    	set( ADDED_CPP_PATTERNS TemplateiOS/*.cpp TemplateiOS/*.mm )
    	set( ADDED_H_PATTERNS TemplateiOS/*.h )

        set(INCLUDES_DIR  ${INCLUDES_DIR} "${CMAKE_CURRENT_LIST_DIR}/TemplateiOS/" PARENT_SCOPE)
        set(INCLUDES_DIR  ${INCLUDES_DIR} "${CMAKE_CURRENT_LIST_DIR}/" PARENT_SCOPE)
    else()
        set( ADDED_CPP_PATTERNS TemplateMacOS/*.cpp )
    	set( ADDED_H_PATTERNS TemplateMacOS/*.h )
    endif()

else()
    set( ADDED_CPP_PATTERNS TemplateWin32/*.cpp )
    set( ADDED_H_PATTERNS TemplateWin32/*.h )
endif()

define_source_files (
GLOB_CPP_PATTERNS *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS *.h     ${ADDED_H_PATTERNS}
PARENT_SCOPE)

