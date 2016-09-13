
set(  MAIN_MODULE_VALUES 
NAME_MODULE                            #
NAME_MODULE_STUB                       #
MODULE_TYPE                            #"[ INLINE STATIC DYNAMIC ]"
#
IMPL_MODULE
EXTERNAL_MODULES
EXTERNAL_MODULES_${DAVA_PLATFORM_CURENT} 
#
MODULE_INITIALIZATION_CODE
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
ERASE_FILES                
ERASE_FILES_${DAVA_PLATFORM_CURENT}     
ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT} 
#
UNITY_IGNORE_LIST             
UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURENT}  
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
#
FIND_SYSTEM_LIBRARY                   
FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURENT}
#
FIND_PACKAGE
FIND_PACKAGE_${DAVA_PLATFORM_CURENT}
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
EXCLUDE_FROM_ALL
#
)
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

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/ModulesInfoCmake.in
                    ${TMP_CMAKE_MODULE_INFO}/CMakeLists.txt  @ONLY )

    if( CUSTOM_DAVA_CONFIG_PATH )
        set( CUSTOM_VALUES -DCUSTOM_DAVA_CONFIG_PATH=${CUSTOM_DAVA_CONFIG_PATH} )
    endif()

    execute_process( COMMAND ${CMAKE_COMMAND} ${TMP_CMAKE_MODULE_INFO}  -DMODULES_TREE_INFO=true -D${DAVA_PLATFORM_CURENT}=true ${CUSTOM_VALUES}
                     WORKING_DIRECTORY ${TMP_CMAKE_MODULE_INFO_BUILD} )

    include( ${TMP_CMAKE_MODULE_INFO}/ModulesInfo.cmake )

endmacro()
#
macro( modules_tree_info )
    if( NAME_MODULE )
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

    endif()

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
                           IGNORE_ITEMS  ${ERASE_FOLDERS_DIR_NAME} ${ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}_DIR_NAME} )
                                     
            set_project_files_properties( "${PROJECT_SOURCE_FILES_CPP}" )
            list( APPEND ALL_SRC  ${PROJECT_SOURCE_FILES} )
            list( APPEND ALL_SRC_HEADER_FILE_ONLY  ${PROJECT_HEADER_FILE_ONLY} )
        endif()

        foreach( VALUE ${MAIN_MODULE_VALUES} )
            set(  ${VALUE} ${${VALUE}_DIR_NAME} )
        endforeach()
    endif()
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
        string(REGEX REPLACE ";" "" IMODULE_INCLUDES ${IMODULE_INCLUDES} )

        foreach( ITEM ${MODULES_INITIALIZATION} )
            list( APPEND IMODULE_CODE_LIST "        IModule* _${ITEM} = new ${ITEM}()\;\n" )
            list( APPEND IMODULE_CODE_LIST "        listModules.emplace_back( _${ITEM} )\;\n" )                
            list( APPEND IMODULE_CODE_LIST "        _${ITEM}->Init()\;\n\n" )
        endforeach()
        
        foreach( ITEM ${MODULES_INITIALIZATION} )
            list( APPEND IMODULE_CODE_LIST "        _${ITEM}->PostInit()\;\n" )
        endforeach()

        foreach( ITEM ${IMODULE_CODE_LIST} )
            set( IMODULE_CODE "${IMODULE_CODE}${ITEM}")
        endforeach()

        set( IMODULE_CPP ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MODULE_MANAGER_TEMPLATE_NAME_WE}.cpp )
        configure_file( ${MODULE_MANAGER_TEMPLATE}
                        ${IMODULE_CPP}  @ONLY ) 
        list( APPEND CPP_FILES ${IMODULE_CPP} )


        file(RELATIVE_PATH RELATIVE_PATH ${CMAKE_CURRENT_LIST_DIR} ${MODULE_MANAGER_TEMPLATE}) 
        string(REGEX REPLACE "/" "\\\\" RELATIVE_PATH ${RELATIVE_PATH})
        get_filename_component( FOLDER_NAME ${RELATIVE_PATH}  DIRECTORY    )

        set( MODULE_GROUP_STRINGS "${FOLDER_NAME} ${IMODULE_CPP}")
    endif()
endmacro()
#
macro( reset_MAIN_MODULE_VALUES )
    foreach( VALUE ${MAIN_MODULE_VALUES} TARGET_MODULES_LIST 
                                         QT_DEPLOY_LIST_VALUE 
                                         QT_LINKAGE_LIST 
                                         QT_LINKAGE_LIST_VALUE 
                                         DEPENDENT_LIST )
        set( ${VALUE} )
        set_property( GLOBAL PROPERTY ${VALUE} ${${VALUE}} )
    endforeach()
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
    get_property( DAVA_COMPONENTS GLOBAL PROPERTY  DAVA_COMPONENTS )
    list (FIND DAVA_COMPONENTS "ALL" _index)
    if ( ${_index} GREATER -1 AND NOT EXCLUDE_FROM_ALL)
        set( INIT true )
    else()
        if( ORIGINAL_NAME_MODULE )
            list (FIND DAVA_COMPONENTS ${ORIGINAL_NAME_MODULE} _index)
            if ( ${_index} GREATER -1)
                set( INIT true )
            endif()
        else()
            set( INIT true )
        endif()
    endif()  

    if( MODULES_TREE_INFO AND INIT )
        modules_tree_info()
    elseif ( INIT )
        #"hack - find first call"
        get_property( MAIN_MODULES_FIND_FIRST_CALL_LIST GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST )
        if( NOT MAIN_MODULES_FIND_FIRST_CALL_LIST )
            modules_tree_info_execute()
            generated_initialization_module_code()
        endif()

        list( APPEND MAIN_MODULES_FIND_FIRST_CALL_LIST "call" )
        set_property(GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST ${MAIN_MODULES_FIND_FIRST_CALL_LIST} )        
    endif()

    if ( INIT AND NOT MODULES_TREE_INFO )
        if( IOS AND ${MODULE_TYPE} STREQUAL "DYNAMIC" )
            set( MODULE_TYPE "STATIC" )
        endif()

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
                           UNITY_IGNORE_LIST )
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
            list( APPEND STATIC_LIBRARIES_WIN         ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
            list( APPEND STATIC_LIBRARIES_WIN_RELEASE ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_RELEASE} ) 
            list( APPEND STATIC_LIBRARIES_WIN_DEBUG   ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_DEBUG} )
            list( APPEND DYNAMIC_LIBRARIES_WIN        ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
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
            find_package( ${NAME} )
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

        define_source( SOURCE         ${CPP_FILES} ${CPP_FILES_${DAVA_PLATFORM_CURENT}}
                                      ${HPP_FILES} ${HPP_FILES_${DAVA_PLATFORM_CURENT}}
                       SOURCE_RECURSE ${CPP_FILES_RECURSE} ${CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}
                                      ${HPP_FILES_RECURSE} ${HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}
                       IGNORE_ITEMS   ${ERASE_FILES} ${ERASE_FILES_${DAVA_PLATFORM_CURENT}}
                       GROUP_STRINGS  ${MODULE_GROUP_STRINGS}
                     )


        list( APPEND ALL_SRC  ${PROJECT_SOURCE_FILES} )
        list( APPEND ALL_SRC_HEADER_FILE_ONLY  ${PROJECT_HEADER_FILE_ONLY} )

        set_project_files_properties( "${ALL_SRC}" )

        if( COVERAGE AND TARGET_FOLDERS_${PROJECT_NAME} AND  NOT ( ${MODULE_TYPE} STREQUAL "INLINE" ) )
            string(REPLACE ";" " " TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )
            string(REPLACE "\"" "" TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )
            list( APPEND DEFINITIONS -DTARGET_FOLDERS_${PROJECT_NAME}="${TARGET_FOLDERS_${PROJECT_NAME}}" )
        endif()

        #"SAVE PROPERTY"
        save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT}
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}          
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
                )

        load_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT}
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
                STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT}
                INCLUDES
                INCLUDES_PRIVATE
                )

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

        if( ${MODULE_TYPE} STREQUAL "INLINE" )
            set (${DIR_NAME}_PROJECT_SOURCE_FILES_CPP ${PROJECT_SOURCE_FILES_CPP} PARENT_SCOPE)
            set (${DIR_NAME}_PROJECT_SOURCE_FILES_HPP ${PROJECT_SOURCE_FILES_HPP} PARENT_SCOPE)
        else()
            project( ${NAME_MODULE} )
            
            generated_unity_sources( ALL_SRC  IGNORE_LIST ${UNITY_IGNORE_LIST}
                                              IGNORE_LIST_${DAVA_PLATFORM_CURENT} ${UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURENT}} ) 
                               
            if( ${MODULE_TYPE} STREQUAL "STATIC" )
                add_library( ${NAME_MODULE} STATIC  ${ALL_SRC} ${ALL_SRC_HEADER_FILE_ONLY} )
                append_property( TARGET_MODULES_LIST ${NAME_MODULE} )  
            elseif( ${MODULE_TYPE} STREQUAL "DYNAMIC" )
                add_library( ${NAME_MODULE} SHARED  ${ALL_SRC} ${ALL_SRC_HEADER_FILE_ONLY} )
                load_property( PROPERTY_LIST TARGET_MODULES_LIST )
                append_property( TARGET_MODULES_LIST ${NAME_MODULE} )            
                add_definitions( -DDAVA_MODULE_EXPORTS )                

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

            endif()

            file_tree_check( "${CMAKE_CURRENT_LIST_DIR}" )

            if( TARGET_FILE_TREE_FOUND )
                add_dependencies(  ${NAME_MODULE} FILE_TREE_${NAME_MODULE} )
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

            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG )
            reset_property( STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} )
            reset_property( INCLUDES_PRIVATE )

            if ( WINDOWS_UAP )
                set_property(TARGET ${NAME_MODULE} PROPERTY VS_MOBILE_EXTENSIONS_VERSION ${WINDOWS_UAP_MOBILE_EXT_SDK_VERSION} )
            endif()
                
        endif()

        #"hack - find first call"
        get_property( MAIN_MODULES_FIND_FIRST_CALL_LIST GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST )
        list( REMOVE_AT  MAIN_MODULES_FIND_FIRST_CALL_LIST 0 )
        set_property(GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST ${MAIN_MODULES_FIND_FIRST_CALL_LIST} )        
        
        list( LENGTH MAIN_MODULES_FIND_FIRST_CALL_LIST LENGTH_DEFINE_SOURCE_LIST  )
        if ( NOT LENGTH_DEFINE_SOURCE_LIST )
            #"first call"
        endif()

        set_property( GLOBAL PROPERTY MODULES_NAME "${NAME_MODULE}" )

    endif()

endmacro ()



