
#set( NAME_MODULE                )
#set( MODULE_TYPE                )#INLINE STATIC DINAMIC 
#
#set( CPP_FILES                  )
#set( HPP_FILES                  )
#set( CPP_FILES_<PLATFORM>       )
#set( HPP_FILES_<PLATFORM>       )
#
#set( CPP_FILES_RECURSE            )
#set( HPP_FILES_RECURSE            )
#set( CPP_FILES_RECURSE_<PLATFORM> )
#set( HPP_FILES_RECURSE_<PLATFORM> )
#
#set( ERASE_FILES                )
#set( ERASE_FILES_<PLATFORM>     )
#set( ERASE_FILES_NOT_<PLATFORM> )
#
#set( DEFINITIONS                )
#set( DEFINITIONS_<PLATFORM>     )
#set( DEFINITIONS_PRIVATE             )
#set( DEFINITIONS_PRIVATE_<PLATFORM>  )
#
#set( STATIC_LIBRARIES_<PLATFORM>           )
#set( STATIC_LIBRARIES_<PLATFORM>_RELEASE   )
#set( STATIC_LIBRARIES_<PLATFORM>_DEBUG     )
#
#set( DINAMIC_LIBRARIES_<PLATFORM>          )
#set( DINAMIC_LIBRARIES_<PLATFORM>_RELEASE  )
#set( DINAMIC_LIBRARIES_<PLATFORM>_DEBUG    )
#
macro( setup_main_module )

    set( INIT )

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
        if( APPLE )
            foreach( VALUE CPP_FILES 
                           CPP_FILES_RECURSE 
                           ERASE_FILES 
                           ERASE_FILES_NOT
                           DEFINITIONS 
                           DEFINITIONS_PRIVATE )
                if( ${VALUE}_APPLE)
                    list( APPEND ${VALUE}_${DAVA_PLATFORM_CURENT} ${${VALUE}_APPLE} )  
                endif()
            endforeach()

        endif()

        foreach( PLATFORM  ${DAVA_PLATFORM_LIST} )
            if( NOT ${PLATFORM} AND ERASE_FILES_NOT_${PLATFORM} )
                list( APPEND ERASE_FILES ${ERASE_FILES_NOT_${PLATFORM}} ) 
            endif()
        endforeach()


        if( ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT} AND ERASE_FILES )
             list(REMOVE_ITEM ERASE_FILES ${ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT}} )

        endif()

        save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}               
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE   
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG   
                DINAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}          
                DINAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE  
                DINAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
            )
        
        define_source_files (
            GLOB_CPP_PATTERNS          ${CPP_FILES}         ${CPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_H_PATTERNS            ${HPP_FILES}         ${HPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_RECURSE_CPP_PATTERNS  ${CPP_FILES_RECURSE} ${CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}
            GLOB_RECURSE_H_PATTERNS    ${HPP_FILES_RECURSE} ${HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}

            GLOB_ERASE_FILES   ${ERASE_FILES}   ${ERASE_FILES_${DAVA_PLATFORM_CURENT}}
        )

        if( IOS AND ${MODULE_TYPE} STREQUAL "DYNAMIC" )
            set( MODULE_TYPE "STATIC" )
        endif()


        if( DEFINITIONS )
            add_definitions( ${DEFINITIONS} )
        endif()

        if( DEFINITIONS_${DAVA_PLATFORM_CURENT} )
            add_definitions( ${DEFINITIONS_${DAVA_PLATFORM_CURENT}} )
        endif()

        include_directories( ${ADD_INCLUDES_DIR} )   

        if( ${MODULE_TYPE} STREQUAL "INLINE" )
            get_filename_component (DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
            set (${DIR_NAME}_CPP_FILES ${CPP_FILES} PARENT_SCOPE)
            set (${DIR_NAME}_H_FILES ${H_FILES}     PARENT_SCOPE)

        else()
        
            generated_unity_sources( CPP_FILES )                     

            if( ${MODULE_TYPE} STREQUAL "STATIC" )
                append_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} ${NAME_MODULE} )
                add_library( ${NAME_MODULE} STATIC  ${H_FILES}  ${CPP_FILES} )
            elseif( ${MODULE_TYPE} STREQUAL "DYNAMIC" )

                add_definitions( -DDAVA_MODULE_EXPORTS )
                append_property( DINAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT} ${NAME_MODULE} )                
                add_library( ${NAME_MODULE} SHARED  ${H_FILES}  ${CPP_FILES} )
            endif()

            if( DEFINITIONS_PRIVATE )
                add_definitions( ${DEFINITIONS_PRIVATE} )
            endif()

            if( DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT} )
                add_definitions( ${DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT}} )
            endif()

        endif()
    endif()

endmacro ()



