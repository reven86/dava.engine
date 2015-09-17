macro(define_ngt_pathes ngt_root)
    get_filename_component( NGT_ROOT ${ngt_root} ABSOLUTE )
    set                   ( NGT_SRC_ROOT "${NGT_ROOT}/src/core/")
    set                   ( NGT_CORE_PATH "${NGT_SRC_ROOT}lib/")
    set                   ( NGT_PLUGINS_PATH "${NGT_SRC_ROOT}plugins/")
endmacro()

macro(declare_ngt_output_dir output_dir)
    get_filename_component( NGT_CORE_OUTPUT_DIR ${output_dir} ABSOLUTE )
    message("NGT_CORE_OUTPUT - " ${NGT_CORE_OUTPUT_DIR})
endmacro()

macro(get_subdirs_list result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
        set(dirlist "")
        foreach(child ${children})
            if(IS_DIRECTORY ${curdir}/${child})
                list(APPEND dirlist ${child})
            endif()
        endforeach()
    set(${result} ${dirlist})
endmacro()

macro ( define_ngt_lib FOLDER_PATH LIB_NAME)
    set(${LIB_NAME}_PATH ${FOLDER_PATH})
    set(${LIB_NAME}_FOUND 1)
endmacro ()

function(set_binary_dir PROJ_NAME DST_DIR DIR_POSTFIX)
    foreach( CONF_NAME ${CMAKE_CONFIGURATION_TYPES} )
        string( TOUPPER ${CONF_NAME} UP_CONF_NAME )
        set(MODULE_OUTPUT_DIR ${DST_DIR}/${CONF_NAME}/${DIR_POSTFIX})

        set_target_properties ( ${PROJ_NAME} PROPERTIES
                                                RUNTIME_OUTPUT_DIRECTORY_${UP_CONF_NAME} ${MODULE_OUTPUT_DIR}
                                                PDB_OUTPUT_DIRECTORY_${UP_CONF_NAME} ${MODULE_OUTPUT_DIR})

    endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endfunction()

macro (append_ngt_lib LIB_NAME)
    add_subdirectory(${${LIB_NAME}_PATH} "ngt/${LIB_NAME}")
    SET_PROPERTY( TARGET ${LIB_NAME} PROPERTY FOLDER "NGT Core")

    if (NOT NGT_CORE_OUTPUT_DIR)
        message(FATAL_ERROR "marco declare_ngt_output_dir have not been called")
    endif()

    set_binary_dir(${LIB_NAME} ${NGT_CORE_OUTPUT_DIR} "")
endmacro()

macro (append_ngt_plugin LIB_NAME)
    add_subdirectory(${${LIB_NAME}_PATH} "ngt/${LIB_NAME}")
    SET_PROPERTY( TARGET ${LIB_NAME} PROPERTY FOLDER "NGT Plugins")

    if (NOT NGT_CORE_OUTPUT_DIR)
        message(FATAL_ERROR "marco declare_ngt_output_dir have not been called")
    endif()

    set_binary_dir(${LIB_NAME} ${NGT_CORE_OUTPUT_DIR} "plugins")
endmacro()
