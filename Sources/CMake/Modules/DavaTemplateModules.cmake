
set(  MAIN_MODULE_VALUES 
NAME_MODULE                            #
MODULE_TYPE                            #"[ INLINE STATIC DYNAMIC ]"
#
SOURCE_FOLDERS             
ERASE_FOLDERS              
ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}   
#
CPP_FILES                  
HPP_FILES                  
CPP_FILES_${DAVA_PLATFORM_CURENT}       
HPP_FILES_${DAVA_PLATFORM_CURENT}       
#
CPP_FILES_RECURSE            
HPP_FILES_RECURSE            
CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT} 
HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT} 
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
)
#
macro( setup_main_module )

    set( INIT )

    get_filename_component (DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    if( NAME_MODULE )
        get_property( DAVA_COMPONENTS GLOBAL PROPERTY  DAVA_COMPONENTS )
        list (FIND DAVA_COMPONENTS ${NAME_MODULE} _index)
        if ( ${_index} GREATER -1)
            set( INIT true )
        endif()
    else()
        set( INIT true )
    endif()

    if( NOT MODULE_TYPE )
        set( MODULE_TYPE INLINE )
    endif()

    if ( INIT )
        if( IOS AND ${MODULE_TYPE} STREQUAL "DYNAMIC" )
            set( MODULE_TYPE "STATIC" )
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

        if( ANDROID )
            list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}}  )
        endif()

        #"FIND LIBRARY"
        foreach( NAME ${FIND_SYSTEM_LIBRARY} ${FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURENT}} )
            FIND_LIBRARY( ${NAME}_LIBRARY  ${NAME} )

            if( ${NAME}_LIBRARY )
                list ( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}  ${${NAME}_LIBRARY} )
            else()
                find_package( ${NAME} )
                list ( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}  ${${NAME}_LIBRARY} )
            endif()
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

        #"SAVE PROPERTY"
        save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT}
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}          
                )

        load_property( PROPERTY_LIST 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
                )
        

        if( SOURCE_FOLDERS )

            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set( ${VALUE}_DIR_NAME ${${VALUE}} )
                set( ${VALUE}  )
            endforeach()

            define_source_folders  ( SRC_ROOT            ${SOURCE_FOLDERS_DIR_NAME}
                                     ERASE_FOLDERS       ${ERASE_FOLDERS_DIR_NAME} ${ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}_DIR_NAME} )

            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set(  ${VALUE} ${${VALUE}_DIR_NAME} )
            endforeach()

            set_project_files_properties( "${PROJECT_SOURCE_FILES_CPP}" )

            load_property( PROPERTY_LIST   
                    DEFINITIONS
                    DEFINITIONS_${DAVA_PLATFORM_CURENT}
                )

        endif()

        #"DEFINE SOURCE"
        define_source_files (
            GLOB_CPP_PATTERNS          ${CPP_FILES}         ${CPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_H_PATTERNS            ${HPP_FILES}         ${HPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_RECURSE_CPP_PATTERNS  ${CPP_FILES_RECURSE} ${CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}
            GLOB_RECURSE_H_PATTERNS    ${HPP_FILES_RECURSE} ${HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}

            GLOB_ERASE_FILES           ${ERASE_FILES} ${ERASE_FILES_${DAVA_PLATFORM_CURENT}}
        )

        set( ALL_SRC ${CPP_FILES} ${PROJECT_SOURCE_FILES} ${H_FILES} )

        set_project_files_properties( "${ALL_SRC}" )

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
            include_directories( ${INCLUDES} )  
        endif()
        if( INCLUDES_${DAVA_PLATFORM_CURENT} )
            include_directories( ${INCLUDES_${DAVA_PLATFORM_CURENT}} )  
        endif()


        if( ${MODULE_TYPE} STREQUAL "INLINE" )
            set (${DIR_NAME}_CPP_FILES ${CPP_FILES} PARENT_SCOPE)
            set (${DIR_NAME}_H_FILES ${H_FILES}     PARENT_SCOPE)

            save_property( PROPERTY_LIST 
                    STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} 
                    STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE 
                    STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
                    )

        else()
            project( ${NAME_MODULE} )

            generate_source_groups_project ()

            generated_unity_sources( ALL_SRC  IGNORE_LIST ${UNITY_IGNORE_LIST}
                                              IGNORE_LIST_${DAVA_PLATFORM_CURENT} ${UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURENT}} ) 
                               
            if( ${MODULE_TYPE} STREQUAL "STATIC" )
                add_library( ${NAME_MODULE} STATIC  ${ALL_SRC} )
                append_property( TARGET_MODULES_LIST ${NAME_MODULE} )            

            elseif( ${MODULE_TYPE} STREQUAL "DYNAMIC" )
                add_definitions( -DDAVA_MODULE_EXPORTS )
                add_library( ${NAME_MODULE} SHARED  ${ALL_SRC}  )

                load_property( PROPERTY_LIST TARGET_MODULES_LIST )
                list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}  ${TARGET_MODULES_LIST} )
                
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

            if( INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT} )
                include_directories( ${INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT}} ) 
            endif() 

            target_link_libraries  ( ${NAME_MODULE}  ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}} )  

            foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG} )
                target_link_libraries  ( ${NAME_MODULE} debug ${FILE} )
            endforeach ()

            foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE} )
                target_link_libraries  ( ${NAME_MODULE} optimized ${FILE} )
            endforeach () 

        endif()
    endif()

endmacro ()



