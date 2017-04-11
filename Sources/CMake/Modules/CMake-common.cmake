# Only interpret ``if()`` arguments as variables or keywords when unquoted.
#if(NOT (CMAKE_VERSION VERSION_LESS 3.1))
    cmake_policy(SET CMP0054 NEW)
#endif()

function (append_property KEY_PROP  VALUE)
    GET_PROPERTY(PROP_LIST_VALUE GLOBAL PROPERTY ${KEY_PROP} )
    LIST(APPEND PROP_LIST_VALUE ${VALUE} )
    list( REMOVE_DUPLICATES PROP_LIST_VALUE )
    SET_PROPERTY(GLOBAL PROPERTY ${KEY_PROP} "${PROP_LIST_VALUE}")
endfunction()


include ( GlobalVariables      )

if ( DAVA_MEMORY_PROFILER )
    add_definitions( -DDAVA_MEMORY_PROFILING_ENABLE )
    if ( NOT DAVA_MEMORY_PROFILER )
        set ( DAVA_MEMORY_PROFILER 1 )
    endif()
endif()

if( ANDROID )
    find_package( AndroidTools REQUIRED )

    if( WIN32 )
        set( MAKE_PROGRAM ${ANDROID_NDK}/prebuilt/windows-x86_64/bin/make.exe )
    elseif( APPLE )
       set( MAKE_PROGRAM ${ANDROID_NDK}/prebuilt/darwin-x86_64/bin/make )
    endif()

    file( TO_CMAKE_PATH "${MAKE_PROGRAM}" MAKE_PROGRAM )
    set (CMAKE_MAKE_PROGRAM "${MAKE_PROGRAM}" CACHE STRING   "Program used to build from makefiles.")
    mark_as_advanced(CMAKE_MAKE_PROGRAM)

endif()

include ( PlatformSettings     )
include ( MergeStaticLibrarees )
include ( FileTreeCheck        )
include ( DavaTemplate         )
include ( DavaTemplateModules  )
include ( CMakeDependentOption )
include ( CMakeParseArguments  )
include ( UnityBuild           )
include ( Coverage             )
include ( ModuleHelper         )

#
macro ( dava_add_definitions DAVA_DEFINITIONS )
    append_property( GLOBAL_DEFINITIONS "${DAVA_DEFINITIONS};${ARGN}" )
endmacro ()

#
macro ( set_subsystem_console )
    if( WIN32 )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG   "/SUBSYSTEM:CONSOLE" )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE" )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE" )
    endif()
endmacro ()

#
macro ( set_project_files_properties FILES_LIST )
    if( APPLE )
        set_source_files_properties( ${FILES_LIST} PROPERTIES COMPILE_FLAGS "-x objective-c++" )
    endif()
endmacro ()

# Macro for precompiled headers
macro (enable_pch)
    if (WIN32)
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
#
macro( processing_mix_data_dependencies DEPENDENT_TARGET_LIST )

    if( TARGET DATA_COPY_${PROJECT_NAME}  )
        foreach (TARGET_NAME ${DEPENDENT_TARGET_LIST})
            if( TARGET ${TARGET_NAME} )
                add_dependencies( ${TARGET_NAME} DATA_COPY_${PROJECT_NAME} )
            endif()
        endforeach ()
    endif()
endmacro ()
#
macro( processing_mix_data )
    cmake_parse_arguments ( ARG "NOT_DATA_COPY"  "" "" ${ARGN} )

    load_property( PROPERTY_LIST MIX_APP_DATA )
    if( ANDROID )
        set( MIX_APP_DIR ${CMAKE_BINARY_DIR}/assets )
        set( DAVA_DEBUGGER_WORKING_DIRECTORY ${MIX_APP_DIR} )
    elseif( WINDOWS_UAP )
        set( MIX_APP_DIR ${CMAKE_CURRENT_BINARY_DIR}/MixResources )
    elseif( DEPLOY )
        if( NOT DEPLOY_DIR_DATA )
            if( MACOS AND NOT MAC_DISABLE_BUNDLE)
                set( DEPLOY_DIR_DATA ${DEPLOY_DIR}/${PROJECT_NAME}.app/Contents/Resources )
            elseif( IOS )
                set( DEPLOY_DIR_DATA ${DEPLOY_DIR}/${PROJECT_NAME}.app )                
            else()
                set( DEPLOY_DIR_DATA ${DEPLOY_DIR} )
            endif()
        endif()

        set( MIX_APP_DIR ${DEPLOY_DIR_DATA} )     
    else()

        if( NOT MIX_APP_DIR )
            set( MIX_APP_DIR ${CMAKE_CURRENT_BINARY_DIR}/MixResources )
        endif()

        set( DAVA_DEBUGGER_WORKING_DIRECTORY ${MIX_APP_DIR} )
    endif()
    
    get_filename_component( MIX_APP_DIR ${MIX_APP_DIR} ABSOLUTE )

    if( NOT ARG_NOT_DATA_COPY )
        add_custom_target ( DATA_COPY_${PROJECT_NAME} )
    endif()

    foreach( ITEM ${MIX_APP_DATA} )

        string(FIND ${ITEM} "=>" SYMBOL_FOUND)
        if (${SYMBOL_FOUND} MATCHES -1)
            string( REGEX REPLACE " " "" ITEM ${ITEM} )
            string( REGEX REPLACE "=" ";" ITEM ${ITEM} )
            list(GET ITEM 0 GROUP_PATH )
            list(GET ITEM 1 DATA_PATH )

            get_filename_component( DATA_PATH ${DATA_PATH} ABSOLUTE )
            execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${MIX_APP_DIR}/${GROUP_PATH} )
            if( NOT ARG_NOT_DATA_COPY )
                ADD_CUSTOM_COMMAND( TARGET DATA_COPY_${PROJECT_NAME}  
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${DATA_PATH} 
                   ${MIX_APP_DIR}/${GROUP_PATH}
                )
            endif()

        else()
            string( REGEX REPLACE " " "" ITEM ${ITEM} )
            string( REGEX REPLACE "=>" ";" ITEM ${ITEM} )
            list(GET ITEM 0 FILE_PATH )
            list(GET ITEM 1 DATA_PATH )

            if( NOT ARG_NOT_DATA_COPY )
                get_filename_component(FILE_NAME ${FILE_PATH} NAME)

                execute_process( COMMAND ${CMAKE_COMMAND} -E rename ${MIX_APP_DIR}/${FILE_PATH} ${MIX_APP_DIR}/${DATA_PATH}/${FILE_NAME} )
            endif()

        endif()

    endforeach()


    if( WINDOWS_UAP )

        file(GLOB LIST_FOLDER_ITEM  "${MIX_APP_DIR}/*" )
        foreach( ITEM ${LIST_FOLDER_ITEM} )
            if( IS_DIRECTORY ${ITEM} )
                set( APP_DATA ${ITEM} ${APP_DATA} )
            endif()
        endforeach()

    elseif( NOT DEPLOY )
        file(GLOB LIST_FOLDER_ITEM  "${MIX_APP_DIR}/*" )
        foreach( ITEM ${LIST_FOLDER_ITEM} )
            if( IS_DIRECTORY ${ITEM} )

                if( MAC_DISABLE_BUNDLE AND MACOS)                    
                    get_filename_component( FOLDER_NAME ${ITEM}  NAME     )
                    foreach( CONFIGURATION ${CMAKE_CONFIGURATION_TYPES} )
                        foreach( TMP_DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CONFIGURATION} ${CMAKE_CURRENT_BINARY_DIR}/${CONFIGURATION}/Contents/Resources )
                            execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory  ${TMP_DATA_DIR} )
                            if( NOT EXISTS ${TMP_DATA_DIR}/${FOLDER_NAME} )
                                execute_process( COMMAND ln -s ${MIX_APP_DIR}/${FOLDER_NAME} ${TMP_DATA_DIR}/${FOLDER_NAME}  )
                            endif()
                        endforeach()
                    endforeach()
                else()
                    list( APPEND RESOURCES_LIST  ${ITEM}  )
                endif()
            endif()
        endforeach()
    endif()

    if( NOT ARG_NOT_DATA_COPY )
        reset_property ( MIX_APP_DATA )
    endif()

endmacro ()

macro(grab_libs OUTPUT_LIST_VAR LIB_LIST EXCLUDE_LIBS ADDITIONAL_LIBS)
    set(OUTPUT_LIST "")
    foreach (LIB_FILE ${LIB_LIST})
        get_filename_component(LIB_NAME ${LIB_FILE} NAME)
        list (FIND ${EXCLUDE_LIBS} ${LIB_NAME} LIB_INDEX)
        if (${LIB_INDEX} EQUAL -1)
            list ( APPEND OUTPUT_LIST ${LIB_FILE}  )
        endif()
    endforeach()
    list (APPEND OUTPUT_LIST ${${ADDITIONAL_LIBS}})
    set(${OUTPUT_LIST_VAR} ${OUTPUT_LIST})
endmacro()

##
#in
#   SOURCE
#   SOURCE_RECURSE
#   IGNORE_ITEMS
#   GROUP_SOURCE
#out
#   PROJECT_SOURCE_FILES
#   PROJECT_SOURCE_FILES_HPP
#   PROJECT_SOURCE_FILES_CPP
#   PROJECT_FOLDERS
#   PROJECT_HEADER_FILE_ONLY
macro (define_source)
    cmake_parse_arguments ( ARG ""  "RECURSIVE_CALL" "SOURCE;SOURCE_RECURSE;GROUP_SOURCE;GROUP_STRINGS;IGNORE_ITEMS" ${ARGN} )
    
    get_property( DEFINE_SOURCE_LIST GLOBAL PROPERTY DEFINE_SOURCE_LIST )
    list( APPEND DEFINE_SOURCE_LIST "define_source" )
    set_property(GLOBAL PROPERTY DEFINE_SOURCE_LIST ${DEFINE_SOURCE_LIST} ) 
    
    if( NOT ARG_RECURSIVE_CALL )
        set( PROJECT_SOURCE_FILES )
        set( PROJECT_SOURCE_FILES_CPP )
        set( PROJECT_SOURCE_FILES_HPP )
        set( PROJECT_HEADER_FILE_ONLY )
        
        if( NOT ARG_SOURCE AND NOT ARG_SOURCE_RECURSE )
            set( ARG_SOURCE ${CMAKE_CURRENT_LIST_DIR} )
        endif()
        
        list( LENGTH DEFINE_SOURCE_LIST LENGTH_DEFINE_SOURCE_LIST  )
        
        if( LENGTH_DEFINE_SOURCE_LIST EQUAL 1 )
            foreach ( ITEM_ARG_SOURCE ${ARG_SOURCE} ${ARG_SOURCE_RECURSE} )
                get_filename_component( ITEM_ARG_SOURCE ${ITEM_ARG_SOURCE} ABSOLUTE )
                if( IS_DIRECTORY ${ITEM_ARG_SOURCE} ) 
                    set( FOLDER_NAME ${ITEM_ARG_SOURCE} )
                else()
                    get_filename_component( FOLDER_NAME ${ITEM_ARG_SOURCE}  DIRECTORY    )
                endif()
                append_property( PROJECT_FOLDERS ${FOLDER_NAME} )
                list(APPEND TARGET_FOLDERS_${PROJECT_NAME} ${FOLDER_NAME} )
            endforeach ()
        endif()
    endif()
    
    foreach ( ITEM_ARG_SOURCE_RECURSE ${ARG_SOURCE_RECURSE} )
        get_filename_component( ITEM_ARG_SOURCE_RECURSE ${ITEM_ARG_SOURCE_RECURSE} ABSOLUTE )
        file( GLOB_RECURSE LIST_SOURCE_RECURSE ${ITEM_ARG_SOURCE_RECURSE} )
        list( APPEND ARG_SOURCE ${LIST_SOURCE_RECURSE} )
    endforeach ()
    
    set( FILE_EXTENSIONS_CPP .c .cc .cpp )
    set( FILE_EXTENSIONS_HPP .h .hpp )        
    if( APPLE )
        list( APPEND FILE_EXTENSIONS_CPP .m .mm )
    endif()  
    
    foreach ( ITEM_ARG_SOURCE ${ARG_SOURCE} )
        get_filename_component( ITEM_ARG_SOURCE ${ITEM_ARG_SOURCE} ABSOLUTE )
        get_filename_component( FOLDER_NAME ${ITEM_ARG_SOURCE}  NAME_WE     )
        
        if( IS_DIRECTORY ${ITEM_ARG_SOURCE} )
            file( GLOB FIND_CMAKELIST "${ITEM_ARG_SOURCE}/CMakeLists.txt")
            if( FIND_CMAKELIST AND ARG_RECURSIVE_CALL )
                set (${FOLDER_NAME}_PROJECT_SOURCE_FILES_CPP )
                set (${FOLDER_NAME}_PROJECT_SOURCE_FILES_HPP )
                add_subdirectory ( ${ITEM_ARG_SOURCE} )
                list( APPEND PROJECT_SOURCE_FILES_CPP ${${FOLDER_NAME}_PROJECT_SOURCE_FILES_CPP}  )
                list( APPEND PROJECT_SOURCE_FILES_HPP ${${FOLDER_NAME}_PROJECT_SOURCE_FILES_HPP}    )
                list( APPEND PROJECT_SOURCE_FILES ${${FOLDER_NAME}_PROJECT_SOURCE_FILES_HPP} ${${FOLDER_NAME}_PROJECT_SOURCE_FILES_CPP} )
            else()
                file( GLOB LIST_SOURCE ${ITEM_ARG_SOURCE}/* )
                define_source( SOURCE ${LIST_SOURCE} IGNORE_ITEMS ${ARG_IGNORE_ITEMS} RECURSIVE_CALL true GROUP_SOURCE ${ARG_GROUP_SOURCE} )
            endif()
        else()
            string(FIND ${ITEM_ARG_SOURCE} "*" MASK_SYMBOL_FOUND)
            if (${MASK_SYMBOL_FOUND} MATCHES -1)
                set( LIST_SOURCE ${ITEM_ARG_SOURCE} )
            else()
                file( GLOB LIST_SOURCE ${ITEM_ARG_SOURCE} )
            endif()

            foreach ( ITEM_LIST_SOURCE ${LIST_SOURCE} )
                get_filename_component( ITEM_EXT ${ITEM_LIST_SOURCE} EXT )
                set( IGNORE_FLAG )
                
                foreach( IGNORE_MASK ${ARG_IGNORE_ITEMS} )
                    if( ${ITEM_LIST_SOURCE} MATCHES ${IGNORE_MASK} )
                        set( IGNORE_FLAG true )
                        break()
                    endif()
                endforeach()
                
                if( NOT IGNORE_FLAG AND ITEM_EXT)
                    foreach( EXT CPP HPP )
                        list (FIND FILE_EXTENSIONS_${EXT} ${ITEM_EXT} _index)
                        if (${_index} GREATER -1)
                            list( APPEND PROJECT_SOURCE_FILES_${EXT} ${ITEM_LIST_SOURCE} )
                            list( APPEND PROJECT_SOURCE_FILES ${ITEM_LIST_SOURCE} )
                        endif() 
                    endforeach()
                endif()
            endforeach ()
        endif()
    endforeach ()

    source_group( "" FILES ${PROJECT_SOURCE_FILES} )
    
    get_property( DEFINE_SOURCE_LIST GLOBAL PROPERTY DEFINE_SOURCE_LIST )
    list( REMOVE_AT  DEFINE_SOURCE_LIST 0 )
    set_property(GLOBAL PROPERTY DEFINE_SOURCE_LIST ${DEFINE_SOURCE_LIST} )        
    
    list( LENGTH DEFINE_SOURCE_LIST LENGTH_DEFINE_SOURCE_LIST  )
    if ( NOT LENGTH_DEFINE_SOURCE_LIST )
        #message( "Project ${CMAKE_CURRENT_LIST_DIR}" )
        get_property( PROJECT_FOLDERS GLOBAL PROPERTY PROJECT_FOLDERS )
        list ( LENGTH PROJECT_FOLDERS LENGTH_PROJECT_FOLDERS )
        if ( ${LENGTH_PROJECT_FOLDERS} GREATER "0" )
           list( REMOVE_DUPLICATES PROJECT_FOLDERS )
        endif()

        set( IGNORE_GROOP_ITEMS )
        if( ARG_GROUP_STRINGS )
            foreach( ITEM ${ARG_GROUP_STRINGS} )
                string(REGEX REPLACE " " ";" FILES ${ITEM} )
                list( GET FILES 0  FILE_GROUP )
                list( REMOVE_AT  FILES 0 )
                source_group( "${FILE_GROUP}" FILES ${FILES} )
                list( APPEND IGNORE_GROOP_ITEMS ${FILES} )
            endforeach () 
        endif()

        foreach ( ITEM ${PROJECT_FOLDERS}  )
            file(RELATIVE_PATH RELATIVE_PATH ${CMAKE_CURRENT_LIST_DIR} ${ITEM})
            #message( "    ${RELATIVE_PATH}")
            #message( "    ${ITEM}")            
            file( GLOB_RECURSE LIST_SOURCE ${ITEM}/* )

            foreach ( ITEM_LIST_SOURCE ${LIST_SOURCE} )

                list (FIND IGNORE_GROOP_ITEMS ${ITEM_LIST_SOURCE} _index)
                if (${_index} MATCHES -1)

                    get_filename_component( ITEM_LIST_SOURCE ${ITEM_LIST_SOURCE} REALPATH )

                    string(REGEX REPLACE "${ITEM}/" "" FILE_GROUP ${ITEM_LIST_SOURCE} )
                    
                    if( RELATIVE_PATH )
                        set( FILE_GROUP ${RELATIVE_PATH}/${FILE_GROUP} )
                    else()
                        set( FILE_GROUP ${FILE_GROUP} )
                    endif()
                    
                    get_filename_component( FILE_GROUP_NAME ${FILE_GROUP} NAME )
                    string(REGEX REPLACE "/" "\\\\" FILE_GROUP ${FILE_GROUP})
                    string(REGEX REPLACE "\\\\${FILE_GROUP_NAME}" "" FILE_GROUP ${FILE_GROUP})
                    string( REGEX REPLACE "^[..\\\\]+" "_EXT_" FILE_GROUP ${FILE_GROUP} )               
                    
                    get_filename_component( FILE_GROUP_EXT ${FILE_GROUP} EXT )
                    if( FILE_GROUP_EXT )
                        source_group( "" FILES ${ITEM_LIST_SOURCE} )
                    else()
                        source_group( "${FILE_GROUP}" FILES ${ITEM_LIST_SOURCE} )
                        #message( "    ${FILE_GROUP}")                     
                    endif()
                endif()

                string(FIND ${ITEM_LIST_SOURCE} ${CMAKE_BINARY_DIR} BINARY_DIR_FOUND)
                if(${BINARY_DIR_FOUND} MATCHES -1)
                    list (FIND PROJECT_SOURCE_FILES ${ITEM_LIST_SOURCE} _index)
                    if (${_index} MATCHES -1 )
                        set_source_files_properties( ${ITEM_LIST_SOURCE} PROPERTIES HEADER_FILE_ONLY TRUE )
                        list( APPEND PROJECT_HEADER_FILE_ONLY ${ITEM_LIST_SOURCE} )
                    endif() 
                endif()

            endforeach ()
            #message( " ")  
        endforeach ()  
        
        foreach ( ITEM_ARG_GROUP_SOURCE ${ARG_GROUP_SOURCE} )
            set( GROUP_PREFIX    ${ITEM_ARG_GROUP_SOURCE} )
            list ( LENGTH ${ITEM_ARG_GROUP_SOURCE} LENGTH_ITEM_ARG_GROUP_SOURCE )
            if( LENGTH_ITEM_ARG_GROUP_SOURCE )
                set( GROUP_FILE_LIST  )
                source_group( "${ITEM_ARG_GROUP_SOURCE}" FILES ${${ITEM_ARG_GROUP_SOURCE}} )
            endif()
        endforeach () 

        #message( "")
        reset_property( PROJECT_FOLDERS )    
    endif()

    if( TARGET_FOLDERS_${PROJECT_NAME} )
        list( REMOVE_DUPLICATES TARGET_FOLDERS_${PROJECT_NAME} )
    endif()

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
    cmake_parse_arguments (ARG "PCH;PARENT_SCOPE" "GROUP" "EXTRA_CPP_FILES;EXTRA_H_FILES;GLOB_RECURSE_CPP_PATTERNS;GLOB_RECURSE_H_PATTERNS;GLOB_CPP_PATTERNS;GLOB_H_PATTERNS;GLOB_ERASE_FILES" ${ARGN})

    # Source files are defined by globbing source files in current source directory and also by including the extra source files if provided
    if (NOT ARG_GLOB_CPP_PATTERNS)
        set (ARG_GLOB_CPP_PATTERNS *.c *.cc *.cpp )    # Default glob pattern
        if( APPLE )  
            list ( APPEND ARG_GLOB_CPP_PATTERNS *.m *.mm )
        endif  ()
    endif ()
    
    if (NOT ARG_GLOB_H_PATTERNS)
        set (ARG_GLOB_H_PATTERNS *.h *.hpp)
    endif ()

    set ( CPP_FILES )
    set ( H_FILES )

    set ( CPP_FILES_RECURSE )
    set ( H_FILES_RECURSE )    

    file( GLOB CPP_FILES ${ARG_GLOB_CPP_PATTERNS} )
    file( GLOB H_FILES ${ARG_GLOB_H_PATTERNS} )

    file( GLOB_RECURSE CPP_FILES_RECURSE ${ARG_GLOB_RECURSE_CPP_PATTERNS} )
    file( GLOB_RECURSE H_FILES_RECURSE ${ARG_GLOB_RECURSE_H_PATTERNS} )    

    list( APPEND CPP_FILES ${ARG_EXTRA_CPP_FILES} )
    list( APPEND H_FILES ${ARG_EXTRA_H_FILES}  )

    list( APPEND CPP_FILES ${CPP_FILES_RECURSE} )
    list( APPEND H_FILES   ${H_FILES_RECURSE} )
      
    set ( SOURCE_FILES ${CPP_FILES} ${H_FILES} )
    
    source_group( "" FILES ${SOURCE_FILES} )

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

    cmake_parse_arguments (ARG "RECURSIVE_CALL" "" "SRC_ROOT;ERASE_FOLDERS" ${ARGN})
    
    IF( NOT ARG_RECURSIVE_CALL )
        set( PROJECT_SOURCE_FILES  ) 
        set( PROJECT_SOURCE_FILES_CPP  ) 
        set( PROJECT_SOURCE_FILES_HPP  ) 
         
        IF( ARG_SRC_ROOT )
            FOREACH( FOLDER_ITEM ${ARG_SRC_ROOT} )
                get_filename_component ( PATH ${FOLDER_ITEM} REALPATH ) 
                list ( APPEND  DAVA_FOLDERS ${PATH} ) 
            ENDFOREACH()
        ELSE()
            list ( APPEND DAVA_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR} ) 
        ENDIF()
        
        set( DAVA_FOLDERS ${DAVA_FOLDERS} PARENT_SCOPE )

    ENDIF()
    
    set( SOURCE_FOLDERS  )
    
    IF( ARG_SRC_ROOT )
    
        FOREACH( FOLDER_ITEM ${ARG_SRC_ROOT} )
            get_filename_component ( FOLDER_ITEM ${FOLDER_ITEM} REALPATH ) 

            set ( CPP_PATTERNS ${FOLDER_ITEM}/*.c ${FOLDER_ITEM}/*.cpp )    
            if( APPLE )  
                list ( APPEND CPP_PATTERNS ${FOLDER_ITEM}/*.m  ${FOLDER_ITEM}/*.mm )
            endif  ()
        
            define_source_files ( GLOB_CPP_PATTERNS ${CPP_PATTERNS}
                                  GLOB_H_PATTERNS   ${FOLDER_ITEM}/*.h ${FOLDER_ITEM}/*.hpp )
                                  
            FILE( GLOB LIST_SOURCE_FOLDERS "${FOLDER_ITEM}/*" )

            list ( APPEND SOURCE_FOLDERS  ${LIST_SOURCE_FOLDERS} ) 
            list ( APPEND PROJECT_SOURCE_FILES_CPP  ${CPP_FILES} ) 
            list ( APPEND PROJECT_SOURCE_FILES_HPP  ${H_FILES}   ) 
            list ( APPEND PROJECT_SOURCE_FILES      ${CPP_FILES} ${H_FILES} )

        ENDFOREACH()

    ELSE()
        define_source_files ( )
        FILE( GLOB SOURCE_FOLDERS "*" )

        list ( APPEND PROJECT_SOURCE_FILES_CPP  ${CPP_FILES} ) 
        list ( APPEND PROJECT_SOURCE_FILES_HPP  ${H_FILES}   ) 
        list ( APPEND PROJECT_SOURCE_FILES      ${CPP_FILES} ${H_FILES} )

    ENDIF()
               
    FOREACH(FOLDER_ITEM ${SOURCE_FOLDERS})
        IF( IS_DIRECTORY "${FOLDER_ITEM}" )
            get_filename_component ( FOLDER_NAME ${FOLDER_ITEM} NAME ) 
            set( NOT_FIND_ERASE_ITEM 1 )
            FOREACH( ERASE_ITEM ${ARG_ERASE_FOLDERS} )
                IF( ${FOLDER_NAME} STREQUAL ${ERASE_ITEM} )
                    set( NOT_FIND_ERASE_ITEM 0 )
                    break()     
                ENDIF()
            ENDFOREACH()
        
            IF( ${NOT_FIND_ERASE_ITEM} )
                FILE(GLOB FIND_CMAKELIST "${FOLDER_ITEM}/CMakeLists.txt")
                IF( FIND_CMAKELIST )
                    if( ${${FOLDER_NAME}_CPP_FILES} )
                        set( ${${FOLDER_NAME}_CPP_FILES} )
                    endif()

                    if( ${${FOLDER_NAME}_H_FILES} )
                        set( ${${FOLDER_NAME}_H_FILES} )
                    endif()

                    add_subdirectory ( ${FOLDER_ITEM} )
                    list ( APPEND PROJECT_SOURCE_FILES ${${FOLDER_NAME}_CPP_FILES} ${${FOLDER_NAME}_H_FILES} )    
                    list ( APPEND PROJECT_SOURCE_FILES_CPP  ${${FOLDER_NAME}_CPP_FILES} ) 
                    list ( APPEND PROJECT_SOURCE_FILES_HPP  ${${FOLDER_NAME}_H_FILES}   ) 
                ELSE()
                    list (APPEND PROJECT_SOURCE_FILES ${CPP_FILES} ${H_FILES})
                    define_source_folders( SRC_ROOT ${FOLDER_ITEM} ERASE_FOLDERS ${ARG_ERASE_FOLDERS} RECURSIVE_CALL )
                ENDIF()
            ENDIF()
        ENDIF()
    ENDFOREACH()
    
endmacro ()

#
macro ( generate_source_groups_project )

    cmake_parse_arguments ( ARG "RECURSIVE_CALL"  "ROOT_DIR;GROUP_PREFIX" "SRC_ROOT;GROUP_FOLDERS" ${ARGN} )

    IF( ARG_ROOT_DIR )
        get_filename_component ( ROOT_DIR ${ARG_ROOT_DIR} REALPATH )

    else()
        set( ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} )

    ENDIF()

    IF( ARG_GROUP_PREFIX )
        set( GROUP_PREFIX  "${ARG_GROUP_PREFIX}\\" )
    else()
        set( GROUP_PREFIX "" )
    ENDIF()


    IF( ARG_SRC_ROOT )
        set( SRC_ROOT_LIST  )

        FOREACH( SRC_ITEM ${ARG_SRC_ROOT} )

            IF( "${SRC_ITEM}" STREQUAL "*" )
                list ( APPEND SRC_ROOT_LIST "*" )
            ELSE()
                get_filename_component ( SRC_ITEM ${SRC_ITEM} REALPATH )
                list ( APPEND SRC_ROOT_LIST ${SRC_ITEM}/* )
            ENDIF()
        ENDFOREACH()

    else()
        set( SRC_ROOT_LIST "*" )

    ENDIF()


    FOREACH( SRC_ROOT_ITEM ${SRC_ROOT_LIST} )

        file ( GLOB_RECURSE FILE_LIST ${SRC_ROOT_ITEM} )

        FOREACH( ITEM ${FILE_LIST} )
            get_filename_component ( FILE_PATH ${ITEM} PATH )

            IF( "${FILE_PATH}" STREQUAL "${ROOT_DIR}" )
                STRING(REGEX REPLACE "${ROOT_DIR}" "" FILE_GROUP ${FILE_PATH} )
            ELSE()
                STRING(REGEX REPLACE "${ROOT_DIR}/" "" FILE_GROUP ${FILE_PATH} )
                STRING(REGEX REPLACE "/" "\\\\" FILE_GROUP ${FILE_GROUP})
            ENDIF()
            source_group( "${GROUP_PREFIX}${FILE_GROUP}" FILES ${ITEM} )

            #message( "<> "${GROUP_PREFIX}" ][ "${FILE_GROUP}" ][ "${ITEM} )
        ENDFOREACH()

    ENDFOREACH()

    IF( NOT ARG_RECURSIVE_CALL )
        FOREACH( GROUP_ITEM ${ARG_GROUP_FOLDERS} )
            if( IS_DIRECTORY "${${GROUP_ITEM}}" )
                generate_source_groups_project( RECURSIVE_CALL GROUP_PREFIX ${GROUP_ITEM}  ROOT_DIR ${${GROUP_ITEM}}  SRC_ROOT ${${GROUP_ITEM}}  )
            else()
                source_group( "${GROUP_ITEM}" FILES ${${GROUP_ITEM}} )
            endif()

        ENDFOREACH()
    ENDIF()

endmacro ()

#
macro(add_target_properties _target _name)
  set(_properties)
  foreach(_prop ${ARGN})
    set(_properties "${_properties} ${_prop}")
  endforeach(_prop)
  get_target_property(_old_properties ${_target} ${_name})
  if(NOT _old_properties)
    # in case it's NOTFOUND
    SET(_old_properties)
  endif(NOT _old_properties)
  set_target_properties(${_target} PROPERTIES ${_name} "${_old_properties} ${_properties}")

endmacro()

#

macro (reset_property KEY_PROP )
    set( ${KEY_PROP} )
    SET_PROPERTY(GLOBAL PROPERTY ${KEY_PROP}  )
endmacro()

macro( load_property  )
    cmake_parse_arguments (ARG "" "" "PROPERTY_LIST" ${ARGN})
    foreach( PROPERTY ${ARG_PROPERTY_LIST} )
        GET_PROPERTY( VALUE GLOBAL PROPERTY  ${PROPERTY} )
        if( VALUE )
            set( ${PROPERTY} ${VALUE} )
            #message( "load prop ${PROPERTY} -> ${VALUE}" )
        endif()
    endforeach()
endmacro()

macro( save_property  )
    cmake_parse_arguments (ARG "" "" "PROPERTY_LIST" ${ARGN})

    foreach( PROPERTY ${ARG_PROPERTY_LIST} )
        if( ${PROPERTY} )
            append_property( ${PROPERTY}  "${${PROPERTY}}" )  
            #message( "append_property - ${PROPERTY} ${${PROPERTY}}") 
        endif()
    endforeach()

endmacro()

macro ( add_content_win_uap_single CONTENT_DIR )

    #get all files from it and add to SRC
    set( CONTENT_LIST)
    set( CONTENT_LIST_TMP)
    file ( GLOB_RECURSE CONTENT_LIST_TMP "${CONTENT_DIR}/*")
    
    #check svn dir (it happens)
    FOREACH( ITEM ${CONTENT_LIST_TMP} )

        STRING ( FIND ${ITEM} ".svn" SVN_DIR_POS )
        if ( ${SVN_DIR_POS} STREQUAL "-1" )
            list ( APPEND CONTENT_LIST ${ITEM} )
        endif()

    ENDFOREACH()
    
    list ( APPEND ADDED_CONTENT_SRC ${CONTENT_LIST} )
    set ( GROUP_PREFIX "Content\\" )
    get_filename_component ( CONTENT_DIR_ABS ${CONTENT_DIR} ABSOLUTE )
    get_filename_component ( CONTENT_DIR_PATH ${CONTENT_DIR_ABS} PATH )

    #process all content files
    FOREACH( ITEM ${CONTENT_LIST} )
        get_filename_component ( ITEM ${ITEM} ABSOLUTE )
        #message("Item: ${ITEM}")
        
        #add item to project source group "Content"
        get_filename_component ( ITEM_PATH ${ITEM} PATH )
        STRING( REGEX REPLACE "${CONTENT_DIR_PATH}" "" ITEM_GROUP ${ITEM_PATH} )
        
        #remove the first '/' symbol
        STRING ( SUBSTRING ${ITEM_GROUP} 0 1 FIRST_SYMBOL )
        if (FIRST_SYMBOL STREQUAL "/")
            STRING ( SUBSTRING ${ITEM_GROUP} 1 -1 ITEM_GROUP )
        endif ()
        
        #reverse the slashes
        STRING( REGEX REPLACE "/" "\\\\" ITEM_GROUP ${ITEM_GROUP} )
        #message( "Group prefix: ${GROUP_PREFIX}" )
        #message( "Item group: ${ITEM_GROUP}" )
        source_group( ${GROUP_PREFIX}${ITEM_GROUP} FILES ${ITEM} )
        
        #set deployment properties to item
        set_property( SOURCE ${ITEM} PROPERTY VS_DEPLOYMENT_CONTENT 1 )
        
        #all resources deploys in specified location
        if ( DAVA_WIN_UAP_RESOURCES_DEPLOYMENT_LOCATION )
            set ( DEPLOYMENT_LOCATION "${DAVA_WIN_UAP_RESOURCES_DEPLOYMENT_LOCATION}\\${ITEM_GROUP}" )
        else ()
            set ( DEPLOYMENT_LOCATION "${ITEM_GROUP}" )
        endif ()
        set_property( SOURCE ${ITEM} PROPERTY VS_DEPLOYMENT_LOCATION ${DEPLOYMENT_LOCATION} )
        
    ENDFOREACH()

endmacro ()

macro ( add_content_win_uap DEPLOYMENT_CONTENT_LIST )

    #process all content files
    FOREACH( ITEM ${DEPLOYMENT_CONTENT_LIST} )
        add_content_win_uap_single ( ${ITEM} )
    ENDFOREACH()

endmacro ()

macro ( add_static_config_libs_win_uap CONFIG_TYPE LIBS_LOCATION OUTPUT_LIB_LIST )

    #take one platform
    list ( GET WINDOWS_UAP_PLATFORMS 0 REF_PLATFORM )
    
    #resolve libs location path
    STRING( REGEX REPLACE "CONFIGURATION_TAG" "${CONFIG_TYPE}" CONCRETE_CONF_LIBS_LOCATION ${LIBS_LOCATION} )
    STRING( REGEX REPLACE "ARCHITECTURE_TAG" "${REF_PLATFORM}" CONCRETE_LIBS_LOCATION ${CONCRETE_CONF_LIBS_LOCATION} )
    
    #find all libs for specified platform
    file ( GLOB REF_LIB_LIST "${CONCRETE_LIBS_LOCATION}/*.lib" )
    
    #find all libs for all platforms
    FOREACH ( LIB_ARCH ${WINDOWS_UAP_PLATFORMS} )
        STRING( REGEX REPLACE "ARCHITECTURE_TAG" "${LIB_ARCH}" CONCRETE_ARCH_LIBS_LOCATION ${CONCRETE_CONF_LIBS_LOCATION} )
        file ( GLOB LIB_LIST "${CONCRETE_ARCH_LIBS_LOCATION}/*.lib" )
        
        #add to list only filenames
        FOREACH ( LIB ${LIB_LIST} )
            get_filename_component ( LIB_FILE ${LIB} NAME )
            list( APPEND LIB_FILE_LIST ${LIB_FILE} )
        ENDFOREACH ()
    ENDFOREACH ()
    
    #unique all platforms' lib list
    list ( LENGTH LIB_FILE_LIST LIB_FILE_LIST_SIZE )
    if ( LIB_FILE_LIST_SIZE )
        list ( REMOVE_DUPLICATES LIB_FILE_LIST )
    endif ()
    
    #compare lists size
    list ( LENGTH REF_LIB_LIST REF_LIB_LIST_SIZE )
    list ( LENGTH LIB_FILE_LIST LIB_FILE_LIST_SIZE )
    unset ( LIB_FILE_LIST )

    #lib sets for all platform must be equal
    if ( NOT REF_LIB_LIST_SIZE STREQUAL LIB_FILE_LIST_SIZE )
        message ( FATAL_ERROR "Equality checking of static lib sets failed. "
                              "Make sure that lib sets are equal for all architectures in ${CONFIG_TYPE} configuration" )
    endif ()
    
    #append every lib to output lib list
    FOREACH ( LIB ${REF_LIB_LIST} )
        #replace platform specified part of path on VS Platform variable
        STRING( REGEX REPLACE "${REF_PLATFORM}" "$(Platform)" LIB ${LIB} )
        list ( APPEND "${OUTPUT_LIB_LIST}_${CONFIG_TYPE}" ${LIB} )
    ENDFOREACH ()

endmacro ()

#search static libs in specified location and add them in ${OUTPUT_LIB_LIST}_DEBUG and ${OUTPUT_LIB_LIST}_RELEASE
#check equality of lib sets for all platforms
#this macro supports different types of lib location, for example project/Libs/Win10/arm/Debug/ or project/Libs/Debug/Win10/arm/
#the first variant is default. LIBS_LOCATION should contain CONFIGURATION_TAG and ARCHITECTURE_TAG for custom libs location
#for example: project/Libs/CONFIGURATION_TAG/Win10/ARCHITECTURE_TAG/
macro ( add_static_libs_win_uap LIBS_LOCATION OUTPUT_LIB_LIST )

    #parse libs location
    STRING ( FIND ${LIBS_LOCATION} "CONFIGURATION_TAG" CONF_TAG_POS )
    if ( NOT ${CONF_TAG_POS} STREQUAL "-1" )
        set ( CONF_TAG_EXIST true )
    endif ()

    STRING ( FIND ${LIBS_LOCATION} "ARCHITECTURE_TAG" ARCH_TAG_POS )
    if ( NOT ${ARCH_TAG_POS} STREQUAL "-1" )
        set ( ARCH_TAG_EXIST true )
    endif ()

    if ( NOT CONF_TAG_EXIST AND NOT ARCH_TAG_EXIST )
        #if no tags, use default variant
        set ( LIBS_LOCATION_FINAL "${LIBS_LOCATION}/ARCHITECTURE_TAG/CONFIGURATION_TAG" )
    elseif ( CONF_TAG_EXIST AND ARCH_TAG_EXIST )
        #all tags are set
        set ( LIBS_LOCATION_FINAL "${LIBS_LOCATION}" )
    else ()
        message ( FATAL_ERROR "Libs location path should contain all tags or no tags: ${LIBS_LOCATION}" )
    endif ()

    add_static_config_libs_win_uap ( "DEBUG"   ${LIBS_LOCATION_FINAL} ${OUTPUT_LIB_LIST} )
    add_static_config_libs_win_uap ( "RELEASE" ${LIBS_LOCATION_FINAL} ${OUTPUT_LIB_LIST} )

endmacro ()

macro ( add_dynamic_config_lib_win_uap CONFIG_TYPE LIBS_LOCATION OUTPUT_LIB_LIST )

    #search dll's 
    FOREACH ( LIB_ARCH ${WINDOWS_UAP_PLATFORMS} )
        file ( GLOB LIB_LIST "${LIBS_LOCATION}/${LIB_ARCH}/${CONFIG_TYPE}/*.dll" )
        list ( APPEND "${OUTPUT_LIB_LIST}_${CONFIG_TYPE}" ${LIB_LIST} )
    ENDFOREACH ()

endmacro ()

macro ( add_dynamic_libs_win_uap LIBS_LOCATION OUTPUT_LIB_LIST )

    add_dynamic_config_lib_win_uap ( "DEBUG"   ${LIBS_LOCATION} ${OUTPUT_LIB_LIST} )
    add_dynamic_config_lib_win_uap ( "RELEASE" ${LIBS_LOCATION} ${OUTPUT_LIB_LIST} )

endmacro ()

#CLEAR -> 1 or 0
#PARAM_PACKER -> -teamcity;-exo;-useCache ...
macro( convert_graphics )
    cmake_parse_arguments (ARG "" "CLEAR;PARAM_PACKER" "" ${ARGN})

    if( NOT ARG_CLEAR  )
        if( DEPLOY )
            set( ARG_CLEAR 1 )
        else()
            set( ARG_CLEAR 0 )
        endif()
    endif()


    if( NOT ARG_PARAM_PACKER AND DEPLOY )
        set( ARG_PARAM_PACKER  "-teamcity" )
    endif()

endmacro()

function (ASSERT VAR_NAME MESSAGE)
    if (NOT ${VAR_NAME})
         message( FATAL_ERROR ${MESSAGE} )
    endif()
endfunction()

function (append_qt5_deploy LIBRARIES)
    GET_PROPERTY(QT_DEPLOY_LIST_VALUE GLOBAL PROPERTY QT_DEPLOY_LIST)
    LIST(APPEND QT_DEPLOY_LIST_VALUE ${${LIBRARIES}})
    SET_PROPERTY(GLOBAL PROPERTY QT_DEPLOY_LIST "${QT_DEPLOY_LIST_VALUE}")
endfunction()

function (set_linkage_qt5_modules LIBRARIES)
    SET_PROPERTY(GLOBAL PROPERTY QT_LINKAGE_LIST ${${LIBRARIES}})
endfunction()

function (get_qt5_deploy_list OUTPUT_VAR_NAME)
    GET_PROPERTY(QT_DEPLOY_LIST_VALUE GLOBAL PROPERTY QT_DEPLOY_LIST)
    LIST(REMOVE_DUPLICATES QT_DEPLOY_LIST_VALUE)
    set(${OUTPUT_VAR_NAME} ${QT_DEPLOY_LIST_VALUE} PARENT_SCOPE)
endfunction()

function (link_with_qt5 TARGET)
    GET_PROPERTY(QT_LINKAGE_LIST_VALUE GLOBAL PROPERTY QT_LINKAGE_LIST)
    target_link_libraries( ${TARGET} ${NO_LINK_WHOLE_ARCHIVE_FLAG} ${QT_LINKAGE_LIST_VALUE} )
endfunction()

function (append_deploy_dependency _PROJECT_NAME)
    GET_PROPERTY(DEPENDENT_LIST GLOBAL PROPERTY DEPLOY_DEPENDENCIES)
    LIST(APPEND DEPENDENT_LIST ${_PROJECT_NAME})
    SET_PROPERTY(GLOBAL PROPERTY DEPLOY_DEPENDENCIES ${DEPENDENT_LIST})
endfunction()

function (get_deploy_dependencies OUTPUT_VAR_NAME)
    GET_PROPERTY(DEPENDENT_LIST GLOBAL PROPERTY DEPLOY_DEPENDENCIES)
    SET(${OUTPUT_VAR_NAME} ${DEPENDENT_LIST} PARENT_SCOPE)
endfunction()
