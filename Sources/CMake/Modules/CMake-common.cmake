# Only interpret ``if()`` arguments as variables or keywords when unquoted.
if(NOT (CMAKE_VERSION VERSION_LESS 3.1))
    cmake_policy(SET CMP0054 NEW)
endif()

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

elseif ( WINDOWS_UAP )

    if ( DAVA_MEMORY_PROFILER )
        message(WARNING "Windows Store platform detected. Memory profiling is disabled")
        remove_definitions( -DDAVA_MEMORY_PROFILING_ENABLE )
        unset ( DAVA_MEMORY_PROFILER )
    endif ()
    
endif()

include ( PlatformSettings     )
include ( MergeStaticLibrarees )
include ( FileTreeCheck        )
include ( DavaTemplate         )
include ( CMakeDependentOption )
include ( CMakeParseArguments  )


set( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

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
        if( APPLE )  
            list ( APPEND ARG_GLOB_CPP_PATTERNS *.m *.mm )
        endif  ()
    endif ()
    
    if (NOT ARG_GLOB_H_PATTERNS)
        set (ARG_GLOB_H_PATTERNS *.h *.hpp)
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

    cmake_parse_arguments (ARG "RECURSIVE_CALL" "" "SRC_ROOT;GLOB_ERASE_FOLDERS" ${ARGN})
    
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

    ENDIF()
    
    set( SOURCE_FOLDERS  )
    
    IF( ARG_SRC_ROOT )
    
        FOREACH( FOLDER_ITEM ${ARG_SRC_ROOT} )

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
            FOREACH( ERASE_ITEM ${ARG_GLOB_ERASE_FOLDERS} )
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
                    define_source_folders( SRC_ROOT ${FOLDER_ITEM} GLOB_ERASE_FOLDERS ${ARG_GLOB_ERASE_FOLDERS} RECURSIVE_CALL )
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
macro ( install_libraries TARGET_NAME )

IF( DAVA_INSTALL )

install(
        TARGETS
        ${TARGET_NAME}
        DESTINATION
        ${DAVA_THIRD_PARTY_LIBRARIES_PATH} )

install(
        DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION
        "${DAVA_THIRD_PARTY_ROOT_PATH}/include/${TARGET_NAME}"
        FILES_MATCHING
        PATTERN
        "*.h" )

install(
        DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION
        "${DAVA_THIRD_PARTY_ROOT_PATH}/include/${TARGET_NAME}"
        FILES_MATCHING
        PATTERN
        "*.hpp" )

ENDIF()

endmacro ()

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

macro ( add_content_win_uap_single CONTENT_DIR )

    #get all files from it and add to SRC
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
        #set ( DEPLOYMENT_LOCATION "${DAVA_WIN_UAP_RESOURCES_DEPLOYMENT_LOCATION}\\${ITEM_GROUP}" )
        set ( DEPLOYMENT_LOCATION "${ITEM_GROUP}" )
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
    
    #find all libs for specified platform
    file ( GLOB REF_LIB_LIST "${LIBS_LOCATION}/${REF_PLATFORM}/${CONFIG_TYPE}/*.lib" )
    
    #find all libs for all platforms
    FOREACH ( LIB_ARCH ${WINDOWS_UAP_PLATFORMS} )
            file ( GLOB LIB_LIST "${LIBS_LOCATION}/${LIB_ARCH}/${CONFIG_TYPE}/*.lib" )
            
            #add to list only filenames
            FOREACH ( LIB ${LIB_LIST} )
                get_filename_component ( LIB_FILE ${LIB} NAME )
                list( APPEND LIB_FILE_LIST ${LIB_FILE} )
            ENDFOREACH ()
    ENDFOREACH ()
    
    #unique all platforms' lib list
    list ( REMOVE_DUPLICATES LIB_FILE_LIST )
    
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
macro ( add_static_libs_win_uap LIBS_LOCATION OUTPUT_LIB_LIST )

    add_static_config_libs_win_uap ( "DEBUG"   ${LIBS_LOCATION} ${OUTPUT_LIB_LIST} )
    add_static_config_libs_win_uap ( "RELEASE" ${LIBS_LOCATION} ${OUTPUT_LIB_LIST} )

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
