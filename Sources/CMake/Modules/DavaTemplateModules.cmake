
#set( NAME_MODULE                )
#set( MODULE_TYPE                )#INLINE STATIC DINAMIC 
#set( CPP_FILES                  )
#set( H_FILES                    )
#set( ERASE_FILES                )
#set( CPP_FILES_<PLATFORM>       )
#set( H_FILES_<PLATFORM>         )
#set( ERASE_FILES_<PLATFORM>     )
#set( DEFINITIONS                )
#set( DEFINITIONS_<PLATFORM>     )
#set( STATIC_LIBRARIES_<PLATFORM>           )
#set( STATIC_LIBRARIES_<PLATFORM>_RELEASE   )
#set( STATIC_LIBRARIES_<PLATFORM>_DEBUG     )
#set( DINAMIC_LIBRARIES_<PLATFORM>          )
#set( DINAMIC_LIBRARIES_<PLATFORM>_RELEASE  )
#set( DINAMIC_LIBRARIES_<PLATFORM>_DEBUG    )
macro( setup_main_module )

    get_property( DAVA_COMPONENTS GLOBAL PROPERTY  DAVA_COMPONENTS )
    list (FIND DAVA_COMPONENTS ${NAME_MODULE} _index)

    if (${_index} GREATER -1)


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
            GLOB_CPP_PATTERNS  ${CPP_FILES}     ${CPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_H_PATTERNS    ${HPP_FILES}     ${HPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_ERASE_FILES   ${ERASE_FILES}   ${ERASE_FILES_${DAVA_PLATFORM_CURENT}}
        )

        if( IOS AND ${MODULE_TYPE} STREQUAL "DINAMIC" )
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
            elseif( ${MODULE_TYPE} STREQUAL "DINAMIC" )
               # append_property( DINAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT} ${NAME_MODULE} )
                add_library( ${NAME_MODULE} SHARED  ${H_FILES}  ${CPP_FILES} )
            endif()
        endif()

    endif()

endmacro ()



