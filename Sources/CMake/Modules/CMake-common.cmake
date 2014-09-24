include ( CMakeDependentOption )
include ( CMakeParseArguments  )
include ( GlobalVariables      )
include ( MergeStaticLibrarees )


# Macro for precompiled headers
macro (enable_pch)
    if (MSVC)
        foreach (FILE ${SOURCE_FILES})
            if (FILE MATCHES \\.cpp$)
                if (FILE MATCHES Precompiled\\.cpp$)
                    set_source_files_properties (${FILE} PROPERTIES COMPILE_FLAGS "/YcPrecompiled.h")
                else ()
                    set_source_files_properties (${FILE} PROPERTIES COMPILE_FLAGS "/YuPrecompiled.h")
                endif ()
            endif ()
        endforeach ()
    else ()
        # TODO: to enable usage of precompiled header in GCC, for now just make sure the correct Precompiled.h is found in the search
        foreach (FILE ${SOURCE_FILES})
            if (FILE MATCHES Precompiled\\.h$)
                get_filename_component (PATH ${FILE} PATH)
                include_directories (${PATH})
                break ()
            endif ()
        endforeach ()
    endif ()
endmacro ()

# Macro for defining source files with optional arguments as follows:
#  GLOB_CPP_PATTERNS <list> - Use the provided globbing patterns for CPP_FILES instead of the default *.cpp
#  GLOB_H_PATTERNS <list> - Use the provided globbing patterns for H_FILES instead of the default *.h
#  EXTRA_CPP_FILES <list> - Include the provided list of files into CPP_FILES result
#  EXTRA_H_FILES <list> - Include the provided list of files into H_FILES result
#  PCH - Enable precompiled header on the defined source files
#  PARENT_SCOPE - Glob source files in current directory but set the result in parent-scope's variable ${DIR}_CPP_FILES and ${DIR}_H_FILES instead
macro (define_source_files)
    # Parse extra arguments
    cmake_parse_arguments (ARG "PCH;PARENT_SCOPE" "GROUP" "EXTRA_CPP_FILES;EXTRA_H_FILES;GLOB_CPP_PATTERNS;GLOB_H_PATTERNS;GLOB_ERASE_FILES" ${ARGN})

    # Source files are defined by globbing source files in current source directory and also by including the extra source files if provided
    if (NOT ARG_GLOB_CPP_PATTERNS)
        set (ARG_GLOB_CPP_PATTERNS *.c *.cpp )    # Default glob pattern
        if( NOT ANDROID )  
            list ( APPEND ARG_GLOB_CPP_PATTERNS *.mm )
        endif  ()
    endif ()
    
    if (NOT ARG_GLOB_H_PATTERNS)
        set (ARG_GLOB_H_PATTERNS *.h)
    endif ()

    file (GLOB CPP_FILES ${ARG_GLOB_CPP_PATTERNS} )
    file (GLOB H_FILES ${ARG_GLOB_H_PATTERNS} )

    list (APPEND CPP_FILES ${ARG_EXTRA_CPP_FILES})
    list (APPEND H_FILES ${ARG_EXTRA_H_FILES})
    set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
    
    # Optionally enable PCH                                                           	                                                                                   	
    if (ARG_PCH)
        enable_pch ()
    endif ()

    if (ARG_PARENT_SCOPE)
        get_filename_component (DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    endif ()

    if ( ARG_GLOB_ERASE_FILES )
        foreach (ERASE_FILE ${ARG_GLOB_ERASE_FILES})
        foreach (FILE_PATH ${H_FILES})
            get_filename_component ( FILE_NAME ${FILE_PATH} NAME)
            if( ${FILE_NAME} STREQUAL  ${ERASE_FILE} )
                list (REMOVE_ITEM H_FILES ${FILE_PATH} )
            endif ()
        endforeach ()
        endforeach ()

        foreach (ERASE_FILE ${ARG_GLOB_ERASE_FILES})
        foreach (FILE_PATH ${CPP_FILES})
            get_filename_component ( FILE_NAME ${FILE_PATH} NAME)
            if( ${FILE_NAME} STREQUAL  ${ERASE_FILE} )
                list (REMOVE_ITEM CPP_FILES ${FILE_PATH} )
            endif ()
        endforeach ()
        endforeach ()
    endif ()
   
    # Optionally accumulate source files at parent scope
    if (ARG_PARENT_SCOPE)
        set (${DIR_NAME}_CPP_FILES ${CPP_FILES} PARENT_SCOPE)
        set (${DIR_NAME}_H_FILES ${H_FILES} PARENT_SCOPE)
    # Optionally put source files into further sub-group (only works for current scope due to CMake limitation)
    endif ()
        
endmacro ()

#
macro (define_source_folders )

    cmake_parse_arguments (ARG "RECURSIVE_CALL" "" "GLOB_FOLDER;GLOB_ERASE_FOLDERS" ${ARGN})
    
    IF( NOT ARG_RECURSIVE_CALL )
        set( PROJECT_SOURCE_FILES  ) 
        set( PROJECT_SOURCE_FILES_CPP  ) 
        set( PROJECT_SOURCE_FILES_HPP  ) 
    ENDIF()
    
    set( SOURCE_FOLDERS  )
    
    IF( ARG_GLOB_FOLDER)
    
        set ( CPP_PATTERNS ${ARG_GLOB_FOLDER}/*.c ${ARG_GLOB_FOLDER}/*.cpp )    
        if( NOT ANDROID )  
            list ( APPEND CPP_PATTERNS ${ARG_GLOB_FOLDER}/*.mm  )
        endif  ()
    
        define_source_files ( GLOB_CPP_PATTERNS ${CPP_PATTERNS}
                              GLOB_H_PATTERNS   ${ARG_GLOB_FOLDER}/*.h )
                              
        FILE( GLOB SOURCE_FOLDERS "${ARG_GLOB_FOLDER}/*" )
    ELSE()
        define_source_files ( )
        FILE( GLOB SOURCE_FOLDERS "*" )
    ENDIF()
    
    list ( APPEND PROJECT_SOURCE_FILES_CPP  ${CPP_FILES} ) 
    list ( APPEND PROJECT_SOURCE_FILES_HPP  ${H_FILES}   ) 
    list ( APPEND PROJECT_SOURCE_FILES      ${CPP_FILES} ${H_FILES} )
             
    FOREACH(FOLDER_ITEM ${SOURCE_FOLDERS})
        IF( IS_DIRECTORY "${FOLDER_ITEM}" )
            get_filename_component ( FOLDER_NAME ${FOLDER_ITEM} NAME ) 
            set( NOT_FIND_ERASE_ITEM 1 )
            FOREACH( ERASE_ITEM ${ARG_GLOB_ERASE_FOLDERS} )
                IF( ${FOLDER_NAME} STREQUAL ${ERASE_ITEM} )
                    set( NOT_FIND_ERASE_ITEM 0 )
                    break()     
                ENDIF()
            ENDFOREACH()
        
            IF( ${NOT_FIND_ERASE_ITEM} )
                FILE(GLOB FIND_CMAKELIST "${FOLDER_ITEM}/CMakeLists.txt")
                IF( FIND_CMAKELIST )
                    add_subdirectory (${FOLDER_NAME})
                    list ( APPEND PROJECT_SOURCE_FILES ${${FOLDER_NAME}_CPP_FILES} ${${FOLDER_NAME}_H_FILES} )    
    		    list ( APPEND PROJECT_SOURCE_FILES_CPP  ${${FOLDER_NAME}_CPP_FILES} ) 
                    list ( APPEND PROJECT_SOURCE_FILES_HPP  ${${FOLDER_NAME}_H_FILES}   ) 
                ELSE()
                    list (APPEND PROJECT_SOURCE_FILES ${CPP_FILES} ${H_FILES})
                    define_source_folders( GLOB_FOLDER ${FOLDER_ITEM} GLOB_ERASE_FOLDERS ${ARG_GLOB_ERASE_FOLDERS} RECURSIVE_CALL )
                ENDIF()
            ENDIF()
        ENDIF()
    ENDFOREACH()

endmacro ()

#
macro ( generate_source_groups_project )

    file (GLOB_RECURSE FILE_LIST "*" )
    
    FOREACH( ITEM ${FILE_LIST} )
        get_filename_component ( FILE_PATH ${ITEM} PATH ) 

        IF( "${FILE_PATH}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}" )
            STRING(REGEX REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" FILE_GROUP ${FILE_PATH} )
        ELSE()
            STRING(REGEX REPLACE "${CMAKE_CURRENT_LIST_DIR}/" "" FILE_GROUP ${FILE_PATH} )
            STRING(REGEX REPLACE "/" "\\\\" FILE_GROUP ${FILE_GROUP})
        ENDIF()

        source_group( "${FILE_GROUP}" FILES ${ITEM} )
    ENDFOREACH()
    
endmacro ()


#
macro ( install_libraries TARGET_NAME )
install(
        TARGETS
        ${TARGET_NAME}
        DESTINATION
        ${DAVA_THIRD_PARTY_LIBRARIES_PATH} )

install(
        DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION
        "D:/Dava/dava.framework/Libs/include/${TARGET_NAME}"
        FILES_MATCHING
        PATTERN
        "*.h")
endmacro ()

















  