include (CMake-common)

set  ( USE_DYNAMIC_CRT 1 CACHE INTERNAL "")

get_filename_component( NGT_ROOT_ABS "${DAVA_ROOT_DIR}/Sources/External/ngt/" ABSOLUTE )
set( NGT_ROOT ${NGT_ROOT_ABS} CACHE INTERNAL "")
set( NGT_SRC_ROOT "${NGT_ROOT}/src/core/" CACHE INTERNAL "")
set( NGT_CORE_PATH "${NGT_SRC_ROOT}lib/" CACHE INTERNAL "")
set( NGT_PLUGINS_PATH "${NGT_SRC_ROOT}plugins/" CACHE INTERNAL "")
set( QML_SCAN_DIR ${DAVA_ROOT_DIR} CACHE INTERNAL "")

file(GLOB RELEASE_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/Release/*.lib")
file(GLOB DEBUG_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/Debug/*.lib")

set(EXCLUDE_LIBS "FColladaVS2010.lib" "libdxt.lib" "libdxtd.lib" CACHE INTERNAL "")
set(ADDITIONAL_RELEASE_LIBS ${RELEASE_LIBS} CACHE INTERNAL "")
set(ADDITIONAL_DEBUG_LIBS ${DEBUG_LIBS} CACHE INTERNAL "")

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

macro (append_ngt_lib LIB_NAME)
    add_subdirectory(${${LIB_NAME}_PATH} "ngt/${LIB_NAME}")
    SET_PROPERTY( TARGET ${LIB_NAME} PROPERTY FOLDER "NGT Core")
endmacro()

macro (append_ngt_plugin LIB_NAME)
    add_subdirectory(${${LIB_NAME}_PATH} "ngt/${LIB_NAME}")
    SET_PROPERTY( TARGET ${LIB_NAME} PROPERTY FOLDER "NGT Plugins")
    append_deploy_dependency(${LIB_NAME})
endmacro()

function (get_ngt_modules LIB_LIST_VAR PLG_LIST_VAR QTLIBS_LIST_VAR)
    list(APPEND NGT_LIBS  wg_types core_command_system core_common core_data_model
                                               core_generic_plugin core_dependency_system core_generic_plugin_manager
                                               core_logging core_logging_system core_qt_common core_qt_script core_reflection
                                               core_reflection_utils core_serialization core_string_utils core_ui_framework core_variant)

    list(APPEND NGT_PLUGINS plg_reflection plg_variant plg_command_system plg_serialization
                                                       plg_file_system plg_qt_common plg_editor_interaction plg_history_ui
                                                       plg_macros_ui plg_qt_app)
    list(APPEND QT_LIBS Core Gui Widgets Concurrent Qml Network Quick QuickWidgets UiTools Xml)

    set (${LIB_LIST_VAR} ${NGT_LIBS} PARENT_SCOPE)
    set (${PLG_LIST_VAR} ${NGT_PLUGINS} PARENT_SCOPE)
    set (${QTLIBS_LIST_VAR} ${QT_LIBS} PARENT_SCOPE)
endfunction()

function (append_ngt_includes)
    include_directories(${NGT_CORE_PATH})
    include_directories(${NGT_SRC_ROOT}/interfaces)
    include_directories(${NGT_SRC_ROOT})
    include_directories(${NGT_SRC_ROOT}/../)
endfunction()

function (configure_ngt _PROJECT_NAME _OUTPUT_PATH)
    append_ngt_includes()
    set (NGT_OUTPUT_DIR ${_OUTPUT_PATH} CACHE INTERNAL "")
    
    # NGT build system use this variable on Mac to resolve path where plugins must be copyed
    set( BW_BUNDLE_NAME ${_PROJECT_NAME} CACHE INTERNAL "")
    set_delayed_deploy_qt()
endfunction()
