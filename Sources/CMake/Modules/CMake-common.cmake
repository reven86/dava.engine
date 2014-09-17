include (CMakeDependentOption)
include (CMakeParseArguments)

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
        set (ARG_GLOB_CPP_PATTERNS *.cpp *.mm)    # Default glob pattern
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

    unset( PROJECT_SOURCE_FILES CACHE ) 
    cmake_parse_arguments (ARG "" "" "GLOB_FOLDER;GLOB_ERASE_FOLDERS" ${ARGN})

    IF( ARG_GLOB_FOLDER)
        define_source_files ( GLOB_CPP_PATTERNS ${ARG_GLOB_FOLDER}/*.cpp ${ARG_GLOB_FOLDER}/*.mm 
                              GLOB_H_PATTERNS   ${ARG_GLOB_FOLDER}/*.h )
        FILE( GLOB SOURCE_FOLDERS "${ARG_GLOB_FOLDER}/*" )
    ELSE()
        define_source_files ( )
        FILE( GLOB SOURCE_FOLDERS "*" )
    ENDIF()
    
    list ( APPEND PROJECT_SOURCE_FILES ${CPP_FILES} ${H_FILES} )
             
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
                ELSE()
                    list (APPEND PROJECT_SOURCE_FILES ${CPP_FILES} ${H_FILES})
                    define_source_folders( GLOB_FOLDER ${FOLDER_ITEM} )
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

  
# This macro creates a true static library bundle with debug and release configurations
# TARGET - the output library, or target, that you wish to contain all of the object files
# CONFIGURATION - DEBUG, RELEASE or ALL
# LIBRARIES - a list of all of the static libraries you want merged into the TARGET
#
# Example use:
#   MERGE_STATIC_LIBRARIES (mytarget ALL "${MY_STATIC_LIBRARIES}")
#
# NOTE: When you call this script, make sure you quote the argument to LIBRARIES if it is a list!
macro (MERGE_STATIC_LIBRARIES TARGET CONFIGURATION LIBRARIES)
	if (WIN32)
		# On Windows you must add aditional formatting to the LIBRARIES variable as a single string for the windows libtool
		# with each library path wrapped in "" in case it contains spaces
		string (REPLACE ";" "\" \"" LIBS "${LIBRARIES}")
		set (LIBS \"${LIBS}\")

		if(${CONFIGURATION} STREQUAL "DEBUG")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS_DEBUG "${LIBS}")
		elseif (${CONFIGURATION} STREQUAL "RELEASE")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS_RELEASE "${LIBS}")
		elseif (${CONFIGURATION} STREQUAL "RELWITHDEBINFO")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS_RELWITHDEBINFO "${LIBS}")
		elseif (${CONFIGURATION} STREQUAL "ALL")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS "${LIBS}")
		else (${CONFIGURATION} STREQUAL "DEBUG")
			message (FATAL_ERROR "Be sure to set the CONFIGURATION argument to DEBUG, RELEASE or ALL")
		endif(${CONFIGURATION} STREQUAL "DEBUG")
	elseif (APPLE AND ${CMAKE_GENERATOR} STREQUAL "Xcode")
		# iOS and OSX platforms with Xcode need slighly less formatting
		string (REPLACE ";" " " LIBS "${LIBRARIES}")

		if(${CONFIGURATION} STREQUAL "DEBUG")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS_DEBUG "${LIBS}")
		elseif (${CONFIGURATION} STREQUAL "RELEASE")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS_RELEASE "${LIBS}")
		elseif (${CONFIGURATION} STREQUAL "RELWITHDEBINFO")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS_RELWITHDEBINFO "${LIBS}")
		elseif (${CONFIGURATION} STREQUAL "ALL")
			set_property (TARGET ${TARGET} APPEND PROPERTY STATIC_LIBRARY_FLAGS "${LIBS}")
		else (${CONFIGURATION} STREQUAL "DEBUG")
			message (FATAL_ERROR "Be sure to set the CONFIGURATION argument to DEBUG, RELEASE or ALL")
		endif(${CONFIGURATION} STREQUAL "DEBUG")
	elseif (UNIX)
		# Posix platforms, including Android, require manual merging of static libraries via a special script
		set (LIBRARIES ${LIBRARIES})

		if (NOT CMAKE_BUILD_TYPE)
			message (FATAL_ERROR "To use the MergeStaticLibraries script on Posix systems, you MUST define your CMAKE_BUILD_TYPE")
		endif (NOT CMAKE_BUILD_TYPE)
		
		set (MERGE OFF)

		# We need the debug postfix on posix systems for the merge script
		string (TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
		if (${BUILD_TYPE} STREQUAL ${CONFIGURATION} OR ${CONFIGURATION} STREQUAL "ALL")
			if (${BUILD_TYPE} STREQUAL "DEBUG")
				get_target_property (TARGETLOC ${TARGET} LOCATION_DEBUG)
			else (${BUILD_TYPE} STREQUAL "DEBUG")
				get_target_property (TARGETLOC ${TARGET} LOCATION)
			endif (${BUILD_TYPE} STREQUAL "DEBUG")
			set (MERGE ON)
		endif (${BUILD_TYPE} STREQUAL ${CONFIGURATION} OR ${CONFIGURATION} STREQUAL "ALL")

		# Setup the static library merge script
		if (NOT MERGE)
			message (STATUS "MergeStaticLibraries ignores mismatch betwen BUILD_TYPE=${BUILD_TYPE} and CONFIGURATION=${CONFIGURATION}")
		else (NOT MERGE)
			configure_file (
				${PROJECT_SOURCE_DIR}/cmake/Modules/PosixMergeStaticLibraries.cmake.in 
				${CMAKE_CURRENT_BINARY_DIR}/PosixMergeStaticLibraries-${TARGET}.cmake @ONLY
			)
			add_custom_command (TARGET ${TARGET} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/PosixMergeStaticLibraries-${TARGET}.cmake
			)
		endif (NOT MERGE)
	endif (WIN32)
endmacro (MERGE_STATIC_LIBRARIES)