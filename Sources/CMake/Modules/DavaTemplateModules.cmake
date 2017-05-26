
set(  MAIN_MODULE_VALUES 
NAME_MODULE                            #
NAME_MODULE_STUB                       #
MODULE_TYPE                            #"[ INLINE STATIC PLUGIN  ]"
#
IMPL_MODULE
EXTERNAL_MODULES
EXTERNAL_MODULES_${DAVA_PLATFORM_CURENT} 
#
MODULE_INITIALIZATION_CODE
MODULE_INITIALIZATION_NAMESPACE
MODULE_MANAGER_TEMPLATE
#
SRC_FOLDERS             
ERASE_FOLDERS              
ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}   
#
CPP_FILES                  
HPP_FILES                  
CPP_FILES_${DAVA_PLATFORM_CURENT}       
HPP_FILES_${DAVA_PLATFORM_CURENT}
#
HPP_FILES_STUB
HPP_FILES_IMPL
HPP_FILES_STUB_${DAVA_PLATFORM_CURENT} 
HPP_FILES_IMPL_${DAVA_PLATFORM_CURENT}   
#
CPP_FILES_STUB
CPP_FILES_IMPL
CPP_FILES_STUB_${DAVA_PLATFORM_CURENT} 
CPP_FILES_IMPL_${DAVA_PLATFORM_CURENT}        
#
CPP_FILES_RECURSE            
HPP_FILES_RECURSE            
CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT} 
HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT} 
GROUP_SOURCE
#
HPP_FILES_RECURSE_STUB
HPP_FILES_RECURSE_IMPL
HPP_FILES_RECURSE_STUB_${DAVA_PLATFORM_CURENT} 
HPP_FILES_RECURSE_IMPL_${DAVA_PLATFORM_CURENT}
CPP_FILES_RECURSE_STUB
CPP_FILES_RECURSE_IMPL
CPP_FILES_RECURSE_STUB_${DAVA_PLATFORM_CURENT} 
CPP_FILES_RECURSE_IMPL_${DAVA_PLATFORM_CURENT}
#
CPP_FILES_EXECUTE
#
ERASE_FILES                
ERASE_FILES_${DAVA_PLATFORM_CURENT}     
ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT} 
#
UNITY_IGNORE_LIST             
UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURENT}
#
CUSTOM_PACK_1
CUSTOM_PACK_1_${DAVA_PLATFORM_CURENT}
#
INCLUDES         
INCLUDES_PRIVATE 
INCLUDES_${DAVA_PLATFORM_CURENT} 
INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT} 
#
DEFINITIONS                
DEFINITIONS_PRIVATE             
DEFINITIONS_${DAVA_PLATFORM_CURENT}     
DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT}  
#
STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}           
STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE   
STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG     
#
DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}           
DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE              
DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG                
#
FIND_SYSTEM_LIBRARY                   
FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURENT}
#
FIND_PACKAGE
FIND_PACKAGE_${DAVA_PLATFORM_CURENT}
#
QT_UI_FILES
QT_RES_FILES
#
DEPLOY_TO_BIN
DEPLOY_TO_BIN_${DAVA_PLATFORM_CURENT}
BINARY_WIN32_DIR_RELEASE
BINARY_WIN32_DIR_DEBUG
BINARY_WIN32_DIR_RELWITHDEB
BINARY_WIN64_DIR_RELEASE
BINARY_WIN64_DIR_DEBUG
BINARY_WIN64_DIR_RELWITHDEB
#
JAR_FOLDERS_ANDROID
JAVA_FOLDERS_ANDROID
#
PLUGIN_OUT_DIR
PLUGIN_OUT_DIR_${DAVA_PLATFORM_CURENT}
#
PLUGIN_RELATIVE_PATH_TO_FOLDER
PLUGIN_COPY_ADD_FILES 
#
DEBUG_POSTFIX
CHECKED_POSTFIX
PROFILE_POSTFIX
RELEASE_POSTFIX
)

macro(apply_default_value VAR DEFAULT_VALUE)
     if (NOT ${VAR})
        set(${VAR} ${DEFAULT_VALUE} PARENT_SCOPE)
     endif()
endmacro()

#
set(  GLOBAL_PROPERTY_VALUES ${MAIN_MODULE_VALUES}  TARGET_MODULES_LIST 
                                                    QT_DEPLOY_LIST_VALUE 
                                                    QT_LINKAGE_LIST 
                                                    QT_LINKAGE_LIST_VALUE 
                                                    DEPENDENT_LIST
                                                    GROUP_SOURCE )
#
macro ( load_external_modules EXTERNAL_MODULES )
    foreach( FOLDER_MODULE ${EXTERNAL_MODULES} )
        file( GLOB FIND_CMAKELIST "${FOLDER_MODULE}/CMakeLists.txt" )
        if( FIND_CMAKELIST )
            get_filename_component ( FOLDER_NAME ${FOLDER_MODULE} NAME )
            add_subdirectory ( ${FOLDER_MODULE}  ${FOLDER_NAME} )       
        else()
            file( GLOB LIST ${FOLDER_MODULE} )
            foreach( ITEM ${LIST} )
                if( IS_DIRECTORY ${ITEM} )
                    load_external_modules( ${ITEM} )
                endif()
            endforeach()
        endif()
    endforeach()
endmacro()
#
macro( modules_tree_info_execute )
    set( TMP_CMAKE_MODULE_INFO       ${CMAKE_CURRENT_BINARY_DIR}/ModulesInfo)
    set( TMP_CMAKE_MODULE_INFO_BUILD ${TMP_CMAKE_MODULE_INFO}/build)

    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${TMP_BUILD_MODULE_INFO}/" )
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${TMP_CMAKE_MODULE_INFO_BUILD}/" )

    set( FOLDER_MODULE ${CMAKE_CURRENT_LIST_DIR} )
    get_property(  ARG_MODULE_COMPONENTS GLOBAL PROPERTY COMPONENTS_${MODULE_COMPONENTS_VALUE_NAME} )

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/ModulesInfoCmake.in
                    ${TMP_CMAKE_MODULE_INFO}/CMakeLists.txt  @ONLY )

    if( CUSTOM_DAVA_CONFIG_PATH )
        set( CUSTOM_VALUE -DCUSTOM_DAVA_CONFIG_PATH=${CUSTOM_DAVA_CONFIG_PATH} )
    endif()

    if( CMAKE_TOOLCHAIN_FILE )
        set( CUSTOM_VALUE_2 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} )
    endif()

    if( QT_VERSION )
        set( CUSTOM_VALUE_3 -DQT_VERSION=${QT_VERSION} )
    endif()

    execute_process( COMMAND ${CMAKE_COMMAND} -G${CMAKE_GENERATOR} ${TMP_CMAKE_MODULE_INFO} -DMODULES_TREE_INFO=true -D${DAVA_PLATFORM_CURENT}=true ${CUSTOM_VALUE} ${CUSTOM_VALUE_2} ${CUSTOM_VALUE_3}
                     WORKING_DIRECTORY ${TMP_CMAKE_MODULE_INFO_BUILD} )

    include( ${TMP_CMAKE_MODULE_INFO}/ModulesInfo.cmake )

endmacro()
#
macro( modules_tree_info )
    if( NAME_MODULE AND ( NOT ${MODULE_TYPE} STREQUAL "PLUGIN" ) )
        append_property( LOADED_MODULES ${NAME_MODULE} )
    endif()

    if( MODULE_INITIALIZATION_CODE AND NAME_MODULE )
        append_property( MODULES_INITIALIZATION ${NAME_MODULE} ) 

        foreach( FILE ${MODULE_INITIALIZATION_CODE} )
            get_filename_component( FILE_EXT ${FILE} EXT )
            if( FILE_EXT STREQUAL ".h" OR FILE_EXT STREQUAL ".hpp")
                get_filename_component( FILE ${FILE} REALPATH )
                append_property( MODULES_INITIALIZATION_HPP ${FILE} ) 
            endif()
        endforeach() 
        
        list( APPEND MODULES_CODE  "set( MODULE_TYPE_${NAME_MODULE} ${MODULE_TYPE} )\n" )            

        if( MODULE_INITIALIZATION_NAMESPACE )
            list( APPEND MODULES_CODE  "set( MODULE_INITIALIZATION_NAMESPACE_${NAME_MODULE} ${MODULE_INITIALIZATION_NAMESPACE} )\n" )
        endif()

        append_property( MODULES_CODE ${MODULES_CODE} ) 

    endif()

    set( EXTERNAL_MODULES ${EXTERNAL_MODULES} ${EXTERNAL_MODULES_${DAVA_PLATFORM_CURENT}} ${IMPL_MODULE} ) 

    if( SRC_FOLDERS OR EXTERNAL_MODULES  )

        foreach( VALUE ${MAIN_MODULE_VALUES} )
            set( ${VALUE}_DIR_NAME ${${VALUE}} )
            set( ${VALUE})
        endforeach()

        if( EXTERNAL_MODULES_DIR_NAME )
            load_external_modules( "${EXTERNAL_MODULES_DIR_NAME}" )
        endif()
        
        if( SRC_FOLDERS_DIR_NAME )
            define_source( SOURCE        ${SRC_FOLDERS_DIR_NAME}
                           IGNORE_ITEMS  ${ERASE_FOLDERS_DIR_NAME} ${ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}_DIR_NAME} )
                                     
            set_project_files_properties( "${PROJECT_SOURCE_FILES_CPP}" )
            list( APPEND ALL_SRC  ${PROJECT_SOURCE_FILES} )
            list( APPEND ALL_SRC_HEADER_FILE_ONLY  ${PROJECT_HEADER_FILE_ONLY} )
        endif()            

        foreach( VALUE ${MAIN_MODULE_VALUES} )
            set(  ${VALUE} ${${VALUE}_DIR_NAME} )
        endforeach()
    endif()

    foreach( NAME ${FIND_PACKAGE} ${FIND_PACKAGE${DAVA_PLATFORM_CURENT}} )
        find_package( ${NAME} COMPONENTS ${MODULE_COMPONENTS} )
    endforeach()

endmacro()
#
macro( generated_initialization_module_code )
    if( MODULE_MANAGER_TEMPLATE )
        get_filename_component( MODULE_MANAGER_TEMPLATE ${MODULE_MANAGER_TEMPLATE} ABSOLUTE )
        get_filename_component( MODULE_MANAGER_TEMPLATE_NAME_WE ${MODULE_MANAGER_TEMPLATE} NAME_WE )

        foreach( ITEM ${MODULES_INITIALIZATION_HPP} )
            file(RELATIVE_PATH RELATIVE_PATH_ITEM ${CMAKE_CURRENT_LIST_DIR} ${ITEM}) 

            list( APPEND IMODULE_INCLUDES "#include \"${RELATIVE_PATH_ITEM}\" \n" )
        endforeach()
        
        if( IMODULE_INCLUDES )
            string(REGEX REPLACE ";" "" IMODULE_INCLUDES ${IMODULE_INCLUDES} )
        endif()

        list( APPEND INIT_POINTERS "struct ModuleManager::PointersToModules\n{\n" )
        foreach( ITEM ${MODULES_INITIALIZATION} )
            set( NAMESPACE_PREFIX )
            if( MODULE_INITIALIZATION_NAMESPACE_${ITEM} )
               set( NAMESPACE_PREFIX "${MODULE_INITIALIZATION_NAMESPACE_${ITEM}}::" )

            endif()

            if( ${MODULE_TYPE_${ITEM}} STREQUAL "INLINE" OR ${MODULE_TYPE_${ITEM}} STREQUAL "STATIC" )
                list( APPEND INIT_POINTERS "    ${NAMESPACE_PREFIX}${ITEM}* _${ITEM}\;\n" )
				list( APPEND GET_MODULE_CODE "template <>\n${NAMESPACE_PREFIX}${ITEM}* ModuleManager::GetModule<${NAMESPACE_PREFIX}${ITEM}>() const\n" )
				list( APPEND GET_MODULE_CODE "{\n    return pointersToModules->_${ITEM}\;\n}\n" )
				list( APPEND CTOR_CODE "    pointersToModules->_${ITEM} = new ${NAMESPACE_PREFIX}${ITEM}(engine)\;\n" )
				list( APPEND CTOR_CODE "    modules.emplace_back(pointersToModules->_${ITEM})\;\n" )   
            elseif( ${MODULE_TYPE_${ITEM}} STREQUAL "PLUGIN" )
            endif()
              
        endforeach()
        list( APPEND INIT_POINTERS "}\;\n" )

        foreach( TYPE_VALUE  INIT_POINTERS GET_MODULE_CODE CTOR_CODE )
            foreach( ITEM ${${TYPE_VALUE}} )
                set( IMODULE_${TYPE_VALUE} "${IMODULE_${TYPE_VALUE}}${ITEM}")
            endforeach()
        endforeach() 

        set( IMODULE_CPP ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MODULE_MANAGER_TEMPLATE_NAME_WE}.cpp )
        configure_file( ${MODULE_MANAGER_TEMPLATE}
                        ${IMODULE_CPP}  @ONLY ) 
        list( APPEND CPP_FILES ${IMODULE_CPP} )
        list( APPEND ERASE_FILES ${MODULE_MANAGER_TEMPLATE_NAME_WE}Stub.cpp )


        file(RELATIVE_PATH RELATIVE_PATH ${CMAKE_CURRENT_LIST_DIR} ${MODULE_MANAGER_TEMPLATE}) 
        string(REGEX REPLACE "/" "\\\\" RELATIVE_PATH ${RELATIVE_PATH})
        get_filename_component( FOLDER_NAME ${RELATIVE_PATH}  DIRECTORY    )

        set( MODULE_GROUP_STRINGS "${FOLDER_NAME} ${IMODULE_CPP}")
    endif()
endmacro()
#
macro( reset_MAIN_MODULE_VALUES )
    foreach( VALUE ${GLOBAL_PROPERTY_VALUES} GLOBAL_DEFINITIONS )
        set( ${VALUE} )
        set_property( GLOBAL PROPERTY ${VALUE} ${${VALUE}} )
    endforeach()
endmacro()
#
macro( dump_module_log  )

    set( MODULES_LOG_FILE  ${CMAKE_BINARY_DIR}/MODULES_LOG.txt )

    get_property( MODULE_CACHE_LOG_LIST GLOBAL PROPERTY MODULE_CACHE_LOG_LIST )

    if( MODULE_CACHE_LOG_LIST )

        set( UNIQUE_COMPONENTS_NUMBER 0 )
        set( USED_UNIQUE_COMPONENTS_NUMBER 0 )

        list( SORT MODULE_CACHE_LOG_LIST )

        file(WRITE ${MODULES_LOG_FILE} "\n" )

        file( APPEND ${MODULES_LOG_FILE} "UNIQUE COMPONENTS LIST\n\n" )
    
        foreach( ITEM ${MODULE_CACHE_LOG_LIST} )
            get_property( CACHE_LOG_${ITEM}_MODULE_UNIQUE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_UNIQUE )

            get_property( CACHE_LOG_${ITEM}_MODULE_CACHE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_CACHE )
            get_property( CACHE_LOG_${ITEM}_MODULE_MD5   GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_MD5 )
            get_property( CACHE_LOG_${ITEM}_MODULE_USES_LIST GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_USES_LIST )

            list(LENGTH CACHE_LOG_${ITEM}_MODULE_USES_LIST CACHE_LOG_${ITEM}_MODULE_USES_LIST_LENGTH )

            if( ${CACHE_LOG_${ITEM}_MODULE_UNIQUE} )
                math( EXPR UNIQUE_COMPONENTS_NUMBER "${UNIQUE_COMPONENTS_NUMBER} + 1" )
                file( APPEND ${MODULES_LOG_FILE} "-> ${UNIQUE_COMPONENTS_NUMBER}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    NAME_MODULE  - ${ITEM} [ ${CACHE_LOG_${ITEM}_MODULE_USES_LIST_LENGTH} ]\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MODULE_CACHE - ${CACHE_LOG_${ITEM}_MODULE_CACHE}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    USES_LIST    - ${CACHE_LOG_${ITEM}_MODULE_USES_LIST}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MD5          - ${CACHE_LOG_${ITEM}_MODULE_MD5}\n" )
                file( APPEND ${MODULES_LOG_FILE} "\n" )
            endif()

        endforeach()

        file( APPEND ${MODULES_LOG_FILE} "\n" )

        file( APPEND ${MODULES_LOG_FILE} "USED UNIQUE COMPONENTS LIST\n\n" )

        foreach( ITEM ${MODULE_CACHE_LOG_LIST} )
            get_property( CACHE_LOG_${ITEM}_MODULE_UNIQUE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_UNIQUE )

            get_property( CACHE_LOG_${ITEM}_MODULE_CACHE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_CACHE )
            get_property( CACHE_LOG_${ITEM}_MODULE_MD5   GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_MD5 )
   
            if( NOT ${CACHE_LOG_${ITEM}_MODULE_UNIQUE} )                
                math( EXPR USED_UNIQUE_COMPONENTS_NUMBER "${USED_UNIQUE_COMPONENTS_NUMBER} + 1" )
                file( APPEND ${MODULES_LOG_FILE} "-> ${USED_UNIQUE_COMPONENTS_NUMBER}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    NAME_MODULE  - ${ITEM}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MODULE_CACHE - ${CACHE_LOG_${ITEM}_MODULE_CACHE}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MD5          - ${CACHE_LOG_${ITEM}_MODULE_MD5}\n" )
                file( APPEND ${MODULES_LOG_FILE} "\n" )
            endif()

        endforeach()

        list(LENGTH MODULE_CACHE_LOG_LIST MODULE_CACHE_LOG_LIST_LENGTH )

        file( APPEND ${MODULES_LOG_FILE} "\n\n" )
        file( APPEND ${MODULES_LOG_FILE} "UNIQUE      - ${UNIQUE_COMPONENTS_NUMBER}\n" )
        file( APPEND ${MODULES_LOG_FILE} "USED_UNIQUE - ${USED_UNIQUE_COMPONENTS_NUMBER}\n" )
        file( APPEND ${MODULES_LOG_FILE} "LIST_LENGTH - ${MODULE_CACHE_LOG_LIST_LENGTH}\n" )

    endif()

endmacro()
#
macro( setup_main_module )

    if( NOT MODULE_TYPE )
        set( MODULE_TYPE INLINE )
    endif()

    set( ORIGINAL_NAME_MODULE ${NAME_MODULE} )

    if( NOT MODULES_TREE_INFO AND NOT ( ${MODULE_TYPE} STREQUAL "INLINE" ) )
        get_property( MODULES_ARRAY GLOBAL PROPERTY MODULES_ARRAY )
        list (FIND MODULES_ARRAY ${NAME_MODULE} _index)
        if ( JOIN_PROJECT_NAME OR ${_index} GREATER -1)
            set( NAME_MODULE ${NAME_MODULE}_${PROJECT_NAME} )
        endif() 
        list( APPEND MODULES_ARRAY ${NAME_MODULE} )
        set_property( GLOBAL PROPERTY MODULES_ARRAY "${MODULES_ARRAY}" )

        project ( ${NAME_MODULE} )
        include ( CMake-common )
    endif()

    set( INIT )

    get_filename_component (DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    set( MODULE_COMPONENTS )

    if( MODULE_COMPONENTS_VALUE_NAME )
        get_property(  MODULE_COMPONENTS GLOBAL PROPERTY COMPONENTS_${MODULE_COMPONENTS_VALUE_NAME} )

        if( ORIGINAL_NAME_MODULE )
            list (FIND MODULE_COMPONENTS ${ORIGINAL_NAME_MODULE} _index)
            if ( ${_index} GREATER -1)
                set( INIT true )
            endif()
        else()
            set( INIT true )
        endif()

    else()
        set( INIT true )
    endif()

    if( MODULES_TREE_INFO AND INIT )
        modules_tree_info()
    elseif ( INIT )
        #"hack - find first call"
        get_property( MAIN_MODULES_FIND_FIRST_CALL_LIST GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST )

        if( NOT MAIN_MODULES_FIND_FIRST_CALL_LIST )            
            modules_tree_info_execute()
        endif()

        if( MODULE_MANAGER_TEMPLATE )            
            generated_initialization_module_code()
        endif()

        list( APPEND MAIN_MODULES_FIND_FIRST_CALL_LIST "call" )
        set_property(GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST ${MAIN_MODULES_FIND_FIRST_CALL_LIST} ) 
    endif()


    if ( INIT AND NOT MODULES_TREE_INFO )
        if( IOS AND ${MODULE_TYPE} STREQUAL "DYNAMIC" )
            set( MODULE_TYPE "STATIC" )
        endif()

#####
        if( ${MODULE_TYPE} STREQUAL "STATIC" )

            if( CPP_FILES_EXECUTE )
                get_filename_component( CPP_FILES_EXECUTE ${CPP_FILES_EXECUTE} ABSOLUTE )
                save_property( PROPERTY_LIST DEFINITIONS CPP_FILES_EXECUTE )
            endif()

            get_property( GLOBAL_DEFINITIONS_PROP GLOBAL PROPERTY GLOBAL_DEFINITIONS )
            get_property( DEFINITIONS_PROP GLOBAL PROPERTY DEFINITIONS )
            get_property( DEFINITIONS_PROP_${DAVA_PLATFORM_CURENT} GLOBAL PROPERTY DEFINITIONS_${DAVA_PLATFORM_CURENT} )

            if( COVERAGE AND MACOS )
                set( COVERAGE_STRING "COVERAGE" )
            else()
                set( COVERAGE_STRING  )                
            endif()

            set( MODULE_CACHE   "ROOT_${ORIGINAL_NAME_MODULE}"
                                ${LOADED_MODULES} 
                                ${DEFINITIONS} 
                                ${DEFINITIONS_${DAVA_PLATFORM_CURENT}}  
                                ${GLOBAL_DEFINITIONS_PROP}
                                ${DEFINITIONS_PROP} 
                                ${DEFINITIONS_PROP_${DAVA_PLATFORM_CURENT}} 
                                ${COVERAGE_STRING} )

            list( REMOVE_DUPLICATES MODULE_CACHE )
            list( SORT MODULE_CACHE )

            append_property( MODULE_CACHE_LOG_LIST ${NAME_MODULE}  )

            set_property( GLOBAL PROPERTY CACHE_LOG_${NAME_MODULE}_MODULE_CACHE  ${MODULE_CACHE} )

            string (REPLACE ";" " " MODULE_CACHE "${MODULE_CACHE}")
            string( MD5  MODULE_CACHE ${MODULE_CACHE} )

            set_property( GLOBAL PROPERTY CACHE_LOG_${NAME_MODULE}_MODULE_MD5  ${MODULE_CACHE}  )
            set_property( GLOBAL PROPERTY CACHE_LOG_${NAME_MODULE}_MODULE_UNIQUE  true )
            set_property( GLOBAL PROPERTY CACHE_LOG_${NAME_MODULE}_MODULE_USES_LIST  )

        endif()
#####            

        if( MODULE_INITIALIZATION_CODE )
            ASSERT( NAME_MODULE "Please define the name of the module in the variable NAME MODULE")
            list( APPEND CPP_FILES ${MODULE_INITIALIZATION_CODE} )
        endif()

        #"APPLE VALUES"
        if( APPLE )
            foreach( VALUE CPP_FILES 
                           CPP_FILES_RECURSE 
                           ERASE_FILES 
                           ERASE_FILES_NOT
                           DEFINITIONS 
                           DEFINITIONS_PRIVATE 
                           INCLUDES
                           INCLUDES_PRIVATE 
                           UNITY_IGNORE_LIST
                           CUSTOM_PACK_1 )
                if( ${VALUE}_APPLE)
                    list( APPEND ${VALUE}_${DAVA_PLATFORM_CURENT} ${${VALUE}_APPLE} )  
                endif()
            endforeach()
        endif()

        #"INCLUDES"
        set( INCLUDES_LIST )
        foreach( ITEM ${INCLUDES} ${INCLUDES_${DAVA_PLATFORM_CURENT}} )
            get_filename_component( ITEM ${ITEM} ABSOLUTE )
            list( APPEND INCLUDES_LIST ${ITEM} )
        endforeach()
        set( INCLUDES  ${INCLUDES_LIST} )
        list( APPEND INCLUDES_PRIVATE  ${INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT}} )
        
        #"STATIC_LIBRARIES"
        if( ANDROID )
            list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}}  )
        endif()
        
        if( WIN )
            list( APPEND STATIC_LIBRARIES_WIN          ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
            list( APPEND STATIC_LIBRARIES_WIN_RELEASE  ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_RELEASE} ) 
            list( APPEND STATIC_LIBRARIES_WIN_DEBUG    ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_DEBUG} )
            list( APPEND DYNAMIC_LIBRARIES_WIN         ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
            list( APPEND DYNAMIC_LIBRARIES_WIN_RELEASE ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_RELEASE} )
            list( APPEND DYNAMIC_LIBRARIES_WIN_DEBUG   ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_DEBUG} )

            foreach( CONFIGURE "_RELEASE" "_DEBUG" )
                foreach( DYNAMIC_LIBRARY ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}${CONFIGURE}} )
                    get_filename_component( DYNAMIC_LIBRARY ${DYNAMIC_LIBRARY} ABSOLUTE )
                    get_filename_component( DYNAMIC_LIBRARY_DIR ${DYNAMIC_LIBRARY}  DIRECTORY )
                    append_property( MODULE_DYNAMIC_LIBRARIES_DIR${CONFIGORE} ${DYNAMIC_LIBRARY_DIR} )  
                endforeach()
            endforeach()

        endif()      
       
        #"FIND LIBRARY"
        foreach( NAME ${FIND_SYSTEM_LIBRARY} ${FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURENT}} )
            FIND_LIBRARY( ${NAME}_LIBRARY  ${NAME} )

            if( ${NAME}_LIBRARY )
                list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} ${${NAME}_LIBRARY} )
            else()
                if ( NOT NOT_TARGET_EXECUTABLE )
                    find_package( ${NAME} )
                    list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} ${${NAME}_LIBRARY} )
                endif()
            endif()
        endforeach()

        #"FIND PACKAGE"
        foreach( NAME ${FIND_PACKAGE} ${FIND_PACKAGE${DAVA_PLATFORM_CURENT}} )
            find_package( ${NAME} COMPONENTS ${MODULE_COMPONENTS} )

            if (PACKAGE_${NAME}_INCLUDES)
                foreach( PACKAGE_INCLUDE ${PACKAGE_${NAME}_INCLUDES} )
                    include_directories(${${PACKAGE_INCLUDE}})
                endforeach()
            endif()
            list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} ${PACKAGE_${NAME}_STATIC_LIBRARIES} )

        endforeach()

        #"ERASE FILES"
        foreach( PLATFORM  ${DAVA_PLATFORM_LIST} )
            if( NOT ${PLATFORM} AND ERASE_FILES_NOT_${PLATFORM} )
                list( APPEND ERASE_FILES ${ERASE_FILES_NOT_${PLATFORM}} ) 
            endif()
        endforeach()
        if( ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT} AND ERASE_FILES )
             list(REMOVE_ITEM ERASE_FILES ${ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT}} )
        endif()

        set( ALL_SRC )
        set( ALL_SRC_HEADER_FILE_ONLY )
        set( EXTERNAL_MODULES ${EXTERNAL_MODULES} ${EXTERNAL_MODULES_${DAVA_PLATFORM_CURENT}} ${IMPL_MODULE} ) 
        
        if( SRC_FOLDERS OR EXTERNAL_MODULES )

            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set( ${VALUE}_DIR_NAME ${${VALUE}} )
                set( ${VALUE})
            endforeach()

            if( EXTERNAL_MODULES_DIR_NAME )
                load_external_modules( "${EXTERNAL_MODULES_DIR_NAME}" )
            endif()
            
            if( SRC_FOLDERS_DIR_NAME )
                define_source( SOURCE        ${SRC_FOLDERS_DIR_NAME}
                                           IGNORE_ITEMS  ${ERASE_FOLDERS_DIR_NAME} ${ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}_DIR_NAME} 
                                                                          ${ERASE_FILES_DIR_NAME} ${ERASE_FILES_${DAVA_PLATFORM_CURENT}_DIR_NAME}
                                            GROUP_SOURCE ${GROUP_SOURCE}
                                         )
                                         
                set_project_files_properties( "${PROJECT_SOURCE_FILES_CPP}" )
                list( APPEND ALL_SRC  ${PROJECT_SOURCE_FILES} )
                list( APPEND ALL_SRC_HEADER_FILE_ONLY  ${PROJECT_HEADER_FILE_ONLY} )
            endif()
 
            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set(  ${VALUE} ${${VALUE}_DIR_NAME} )
            endforeach()

        endif()

        if( NAME_MODULE_STUB )
            set( CONECTION_TYPE STUB )
            list (FIND LOADED_MODULES ${NAME_MODULE_STUB} _index)
            if ( ${_index} GREATER -1 )
                set( NAME_MODULE )
                set( MODULE_TYPE INLINE )
                set( CONECTION_TYPE IMPL )

            endif()

            foreach ( ITEM  HPP_FILES_RECURSE HPP_FILES
                            CPP_FILES_RECURSE CPP_FILES )
                list( APPEND ${ITEM}   ${${ITEM}_${CONECTION_TYPE}} )
                list( APPEND ${ITEM}_${DAVA_PLATFORM_CURENT} ${${ITEM}_${CONECTION_TYPE}_${DAVA_PLATFORM_CURENT}} )
            endforeach ()

        endif()

        if (QT_UI_FILES OR QT_RES_FILES)
            file              ( GLOB_RECURSE UI_LIST  ${QT_UI_FILES})
            qt5_wrap_ui ( QT_UI_HEADERS ${UI_LIST} )

            file              ( GLOB_RECURSE RCC_LIST  ${QT_RES_FILES})
            qt5_add_resources ( QT_RCC  ${RCC_LIST} )

            list(APPEND HPP_FILES ${QT_UI_HEADERS})
            list(APPEND CPP_FILES ${QT_RCC})

            set(QtGenerated ${QT_UI_HEADERS} ${QT_RCC})
            list(APPEND GROUP_SOURCE QtGenerated)
        endif()

        define_source( SOURCE         ${CPP_FILES} ${CPP_FILES_${DAVA_PLATFORM_CURENT}}
                                      ${HPP_FILES} ${HPP_FILES_${DAVA_PLATFORM_CURENT}}
                       SOURCE_RECURSE ${CPP_FILES_RECURSE} ${CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}
                                      ${HPP_FILES_RECURSE} ${HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}
                       IGNORE_ITEMS   ${ERASE_FILES} ${ERASE_FILES_${DAVA_PLATFORM_CURENT}}
                       GROUP_SOURCE ${GROUP_SOURCE}
                       GROUP_STRINGS  ${MODULE_GROUP_STRINGS}
                     )


        list( APPEND ALL_SRC  ${PROJECT_SOURCE_FILES} )
        list( APPEND ALL_SRC_HEADER_FILE_ONLY  ${PROJECT_HEADER_FILE_ONLY} )

        set_project_files_properties( "${ALL_SRC}" )

        #"SAVE PROPERTY"
        save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT}
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}          
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
                STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT}
                DEPLOY_TO_BIN
                DEPLOY_TO_BIN_${DAVA_PLATFORM_CURENT}
                INCLUDES
                INCLUDES_PRIVATE
                BINARY_WIN32_DIR_RELEASE
                BINARY_WIN32_DIR_DEBUG
                BINARY_WIN32_DIR_RELWITHDEB
                BINARY_WIN64_DIR_RELEASE
                BINARY_WIN64_DIR_DEBUG
                BINARY_WIN64_DIR_RELWITHDEB
                JAR_FOLDERS_ANDROID
                JAVA_FOLDERS_ANDROID
                )

        load_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT}
                GLOBAL_DEFINITIONS                
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
                STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT}
                INCLUDES
                INCLUDES_PRIVATE
                )

        list( APPEND DEFINITIONS ${GLOBAL_DEFINITIONS} )

        #"DEFINITIONS"
        if( DEFINITIONS )
            add_definitions( ${DEFINITIONS} )
        endif()
        if( DEFINITIONS_${DAVA_PLATFORM_CURENT} )
            add_definitions( ${DEFINITIONS_${DAVA_PLATFORM_CURENT}} )
        endif()

        #"INCLUDES_DIR"
        load_property( PROPERTY_LIST INCLUDES )
        if( INCLUDES )
            include_directories( "${INCLUDES}" )  
        endif()

        #"PLUGIN_OUT_DIR"
        if( PLUGIN_OUT_DIR_${DAVA_PLATFORM_CURENT} )
            set( PLUGIN_OUT_DIR PLUGIN_OUT_DIR_${DAVA_PLATFORM_CURENT}  )
        endif()

        if( ${MODULE_TYPE} STREQUAL "INLINE" )
            set (${DIR_NAME}_PROJECT_SOURCE_FILES_CPP ${PROJECT_SOURCE_FILES_CPP} PARENT_SCOPE)
            set (${DIR_NAME}_PROJECT_SOURCE_FILES_HPP ${PROJECT_SOURCE_FILES_HPP} PARENT_SCOPE)
        else()


#####
            set( CREATE_NEW_MODULE true )

            if( ${MODULE_TYPE} STREQUAL "STATIC" )
                get_property( MODULE_CACHE_LIST GLOBAL PROPERTY MODULE_CACHE_LIST )
                list (FIND MODULE_CACHE_LIST ${MODULE_CACHE} _index)
                if ( ${_index} GREATER -1 )
                    set( CREATE_NEW_MODULE )
                    list(GET MODULE_CACHE_LIST ${_index}  MODULE_CACHE )
                    get_property( MODULE_CACHE_LOADED_NAME GLOBAL PROPERTY ${MODULE_CACHE} )
                    set_property( GLOBAL PROPERTY CACHE_LOG_${NAME_MODULE}_MODULE_UNIQUE  false )
                    append_property( CACHE_LOG_${MODULE_CACHE_LOADED_NAME}_MODULE_USES_LIST ${NAME_MODULE} )  
                    set( NAME_MODULE ${MODULE_CACHE_LOADED_NAME} )
                endif()
            endif()

######

            if( CREATE_NEW_MODULE )
                project( ${NAME_MODULE} )
                
                generated_unity_sources( ALL_SRC  IGNORE_LIST ${UNITY_IGNORE_LIST}
                                                  IGNORE_LIST_${DAVA_PLATFORM_CURENT} ${UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURENT}}
                                                  CUSTOM_PACK_1 ${CUSTOM_PACK_1} ${CUSTOM_PACK_1_${DAVA_PLATFORM_CURENT}}) 
            endif()

            if( ${MODULE_TYPE} STREQUAL "STATIC" )
                if( CREATE_NEW_MODULE )
                    add_library( ${NAME_MODULE} STATIC  ${ALL_SRC} ${ALL_SRC_HEADER_FILE_ONLY} )
                endif()
                append_property( TARGET_MODULES_LIST ${NAME_MODULE} )  

            elseif( ${MODULE_TYPE} STREQUAL "PLUGIN" )
                add_library( ${NAME_MODULE} SHARED  ${ALL_SRC} ${ALL_SRC_HEADER_FILE_ONLY} )
                append_property( PLUGIN_LIST ${NAME_MODULE} )

                load_property( PROPERTY_LIST TARGET_MODULES_LIST ) 
                list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} ${TARGET_MODULES_LIST} )  
                add_definitions( -DDAVA_IMPLEMENT_PLUGIN_MODULE )  

                if( WIN32 )
                    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG" )
                endif()

                apply_default_value(DEBUG_POSTFIX "Debug")
                apply_default_value(CHECKED_POSTFIX "")
                apply_default_value(PROFILE_POSTFIX "")
                apply_default_value(RELEASE_POSTFIX "")

                set_target_properties( ${NAME_MODULE} PROPERTIES
                                                                 DEBUG_OUTPUT_NAME "${NAME_MODULE}" 
                                                                 DEBUG_POSTFIX ${DEBUG_POSTFIX}
                                                                 CHECKED_POSTFIX ${CHECKED_POSTFIX}
                                                                 PROFILE_POSTFIX ${PROFILE_POSTFIX}
                                                                 RELEASE_POSTFIX ${RELEASE_POSTFIX})

                if( WIN32 AND NOT DEPLOY )
                    set( BINARY_WIN32_DIR_RELEASE    "${CMAKE_CURRENT_BINARY_DIR}/Release" )
                    set( BINARY_WIN32_DIR_DEBUG      "${CMAKE_CURRENT_BINARY_DIR}/Debug" )
                    set( BINARY_WIN32_DIR_RELWITHDEB "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebinfo" )
                    set( BINARY_WIN64_DIR_RELEASE    "${CMAKE_CURRENT_BINARY_DIR}/Release" )
                    set( BINARY_WIN64_DIR_DEBUG      "${CMAKE_CURRENT_BINARY_DIR}/Debug" )
                    set( BINARY_WIN64_DIR_RELWITHDEB "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebinfo" )
                    save_property( PROPERTY_LIST BINARY_WIN32_DIR_RELEASE 
                                                 BINARY_WIN32_DIR_DEBUG
                                                 BINARY_WIN32_DIR_RELWITHDEB
                                                 BINARY_WIN64_DIR_RELEASE 
                                                 BINARY_WIN64_DIR_DEBUG
                                                 BINARY_WIN64_DIR_RELWITHDEB )
                endif()

                if( PLUGIN_OUT_DIR )
                    foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
                        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
                        
                        if( APPLE )
                            set_target_properties( ${NAME_MODULE} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PLUGIN_OUT_DIR} )                
                        else()
                            set_target_properties( ${NAME_MODULE} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PLUGIN_OUT_DIR} )
                        endif()

                    endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
                endif()
                 
                if( PLUGIN_RELATIVE_PATH_TO_FOLDER )
                    set_property( GLOBAL PROPERTY ${NAME_MODULE}_RELATIVE_PATH_TO_FOLDER ${PLUGIN_RELATIVE_PATH_TO_FOLDER} )
                endif()

                if( PLUGIN_COPY_ADD_FILES )
                    set_property( GLOBAL PROPERTY ${NAME_MODULE}_PLUGIN_COPY_ADD_FILES ${PLUGIN_COPY_ADD_FILES} )                    
                endif()

            endif()

            if( CREATE_NEW_MODULE )
                file_tree_check( "${CMAKE_CURRENT_LIST_DIR}" )

                if( TARGET_FILE_TREE_FOUND )
                    add_dependencies(  ${NAME_MODULE} FILE_TREE_${NAME_MODULE} )
                endif()

            endif()


            if( DEFINITIONS_PRIVATE )
                add_definitions( ${DEFINITIONS_PRIVATE} )
            endif()

            if( DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT} )
                add_definitions( ${DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT}} )
            endif()

            if( INCLUDES_PRIVATE )
                include_directories( ${INCLUDES_PRIVATE} ) 
            endif() 

            include_directories( . ) 


            if( CREATE_NEW_MODULE )


                if( WIN32 )
                    grab_libs(LIST_SHARED_LIBRARIES_DEBUG   "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG}"   EXCLUDE_LIBS ADDITIONAL_DEBUG_LIBS)
                    grab_libs(LIST_SHARED_LIBRARIES_RELEASE "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE}" EXCLUDE_LIBS ADDITIONAL_RELEASE_LIBS)
                    set( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG   ${LIST_SHARED_LIBRARIES_DEBUG} )
                    set( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE ${LIST_SHARED_LIBRARIES_RELEASE} )
                endif()

                if( LINK_THIRD_PARTY )                 
                    MERGE_STATIC_LIBRARIES( ${NAME_MODULE} ALL "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}}" )
                    MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} DEBUG "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG}" )
                    MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} RELEASE "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE}" )
                endif()

                target_link_libraries  ( ${NAME_MODULE}  ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}}
                                                         ${STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT}} )  

                foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG} )
                    target_link_libraries  ( ${NAME_MODULE} debug ${FILE} )
                endforeach ()

                foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE} )
                    target_link_libraries  ( ${NAME_MODULE} optimized ${FILE} )
                endforeach ()

                if (QT5_FOUND)
                    link_with_qt5(${PROJECT_NAME})
                endif()

                if( COVERAGE AND MACOS )

                    string(REPLACE ";" " " TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )
                    string(REPLACE "\"" "" TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )

                    add_definitions( -DTEST_COVERAGE )
                    add_definitions( -DDAVA_FOLDERS="${DAVA_FOLDERS}" )
                    add_definitions( -DDAVA_UNITY_FOLDER="${CMAKE_BINARY_DIR}/unity_pack" )

                    list( APPEND EXECUTE_DEFINITIONS -DTARGET_FOLDERS_${ORIGINAL_NAME_MODULE}="${TARGET_FOLDERS_${PROJECT_NAME}}" )

                    append_property( EXECUTE_DEFINITIONS_${NAME_MODULE} "${EXECUTE_DEFINITIONS}" )

                    set_target_properties(${NAME_MODULE} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_TEST_COVERAGE_FILES YES )
                    set_target_properties(${NAME_MODULE} PROPERTIES XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS YES )

                endif()   

                if ( WINDOWS_UAP )
                    set_property(TARGET ${NAME_MODULE} PROPERTY VS_MOBILE_EXTENSIONS_VERSION ${WINDOWS_UAP_MOBILE_EXT_SDK_VERSION} )
                endif()             
            endif()

            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG )
            reset_property( STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} )
            reset_property( INCLUDES_PRIVATE )

                
        endif()

        #"hack - find first call"
        get_property( MAIN_MODULES_FIND_FIRST_CALL_LIST GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST )
        list( REMOVE_AT  MAIN_MODULES_FIND_FIRST_CALL_LIST 0 )
        set_property(GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST ${MAIN_MODULES_FIND_FIRST_CALL_LIST} )        
        
        list( LENGTH MAIN_MODULES_FIND_FIRST_CALL_LIST LENGTH_DEFINE_SOURCE_LIST  )
        if ( NOT LENGTH_DEFINE_SOURCE_LIST )
            #"first call"
            set_property( GLOBAL PROPERTY MODULES_NAME "${NAME_MODULE}" )

        endif()

        if( CREATE_NEW_MODULE AND ${MODULE_TYPE} STREQUAL "STATIC" )
            set_property( GLOBAL PROPERTY ${MODULE_CACHE} "${NAME_MODULE}" )
            append_property(  MODULE_CACHE_LIST ${MODULE_CACHE} )

        endif()


    endif()

endmacro ()



