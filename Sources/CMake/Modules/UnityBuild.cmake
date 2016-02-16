macro( generated_unity_sources SOURCE_FILES )  

    list( REMOVE_DUPLICATES ${SOURCE_FILES} )

    if( UNITY_BUILD )
        message( ">>> Unity packages ${PROJECT_NAME} info" )

        cmake_parse_arguments (ARG "" "" "IGNORE_LIST;IGNORE_LIST_APPLE;IGNORE_LIST_WIN32" ${ARGN})
        
        foreach( TYPE_OS  APPLE IOS MACOS WIN32 ANDROID )
            if( ${TYPE_OS} )
                list( APPEND ARG_IGNORE_LIST ${ARG_IGNORE_LIST_${TYPE_OS}} )
            endif()
        endforeach()

        set( CPP_PACK_SIZE 0      )
        set( CPP_PACK_LIST        )
        set( CPP_LIST_SIZE 0      )
        set( CPP_LIST             )
        set( CPP_ALL_LIST         )
        set( CPP_ALL_LIST_SIZE    )

        set( OBJCPP_PACK_SIZE 0   )
        set( OBJCPP_PACK_LIST     )
        set( OBJCPP_LIST_SIZE 0   )
        set( OBJCPP_LIST          )
        set( OBJCPP_ALL_LIST      )
        set( OBJCPP_ALL_LIST_SIZE )

        set( OBJCPP_PACK_EXP mm   ) 
        set( CPP_PACK_EXP    cpp  ) 

        set( IGNORE_LIST_SIZE 0   )

        set( REMAINING_LIST       )

        foreach( ITEM ${${SOURCE_FILES}} )
            set( IGNORE_FLAG )
            foreach( IGNORE_MASK ${ARG_IGNORE_LIST} )
                if( ${ITEM} MATCHES ${IGNORE_MASK} )
                    set( IGNORE_FLAG true )
                    math( EXPR IGNORE_LIST_SIZE "${IGNORE_LIST_SIZE} + 1" )
                    break()
                endif()
            endforeach()
            get_filename_component( ITEM_EXT ${ITEM} EXT )
            if( NOT IGNORE_FLAG AND "${ITEM_EXT}" STREQUAL ".cpp" )
                list( APPEND CPP_ALL_LIST  ${ITEM} )
            elseif( NOT IGNORE_FLAG AND ( "${ITEM_EXT}" STREQUAL ".m" OR "${ITEM_EXT}" STREQUAL ".mm" ) )
                list( APPEND OBJCPP_ALL_LIST  ${ITEM} )                
            else()
                list( APPEND REMAINING_LIST ${ITEM} )
            endif()
        endforeach()  

        list( LENGTH CPP_ALL_LIST    CPP_ALL_LIST_SIZE )
        list( LENGTH OBJCPP_ALL_LIST OBJCPP_ALL_LIST_SIZE )

        if( NOT UNITY_BUILD_PACKAGES_NUMBER )
            set( UNITY_BUILD_PACKAGES_NUMBER 7 )
        endif()

        if( OBJCPP_ALL_LIST )
            math( EXPR UNITY_BUILD_PACKAGES_NUMBER "${UNITY_BUILD_PACKAGES_NUMBER} - 1" )
        endif()

        foreach( PTYPE CPP OBJCPP )
            if( ${${PTYPE}_ALL_LIST_SIZE} EQUAL 0 )
                continue()
            endif()

            if( ${PTYPE} STREQUAL "CPP" )
                math( EXPR CPP_NUMBER_FILES_IN_PACK "${CPP_ALL_LIST_SIZE} / ${UNITY_BUILD_PACKAGES_NUMBER}" ) 
            else()
                math( EXPR OBJCPP_NUMBER_FILES_IN_PACK "${OBJCPP_ALL_LIST_SIZE}" ) 
            endif( )

            foreach( ITEM ${${PTYPE}_ALL_LIST} )
                list( APPEND ${PTYPE}_LIST  ${ITEM} )
                math( EXPR ${PTYPE}_LIST_SIZE "${${PTYPE}_LIST_SIZE} + 1" )
                if( ${${PTYPE}_LIST_SIZE} GREATER ${${PTYPE}_NUMBER_FILES_IN_PACK} )
                    math( EXPR ${PTYPE}_PACK_SIZE "${${PTYPE}_PACK_SIZE} + 1" )
                    set( ${PTYPE}_PACK_${${PTYPE}_PACK_SIZE} ${${PTYPE}_LIST} )
                    set( ${PTYPE}_LIST )
                    set( ${PTYPE}_LIST_SIZE 0 )                
                endif()
            endforeach()  

            if( ${PTYPE}_LIST_SIZE )
                math( EXPR ${PTYPE}_PACK_SIZE "${${PTYPE}_PACK_SIZE} + 1" )
                set( ${PTYPE}_PACK_${${PTYPE}_PACK_SIZE} ${${PTYPE}_LIST} )

            endif()

            get_property( PACK_IDX GLOBAL PROPERTY  ${PROJECT_NAME}_PACK_IDX  )

            if( NOT PACK_IDX )
                set( PACK_IDX 0 )
            endif()
        
            foreach( index RANGE 1 ${${PTYPE}_PACK_SIZE}  )
                set( HEADERS_LIST )
                foreach( PACH ${${PTYPE}_PACK_${index}} )
                    get_filename_component( PACH ${PACH} ABSOLUTE )
                    list( APPEND HEADERS_LIST "#include\"${PACH}\"" ) 
                    set_source_files_properties( ${PACH} PROPERTIES HEADER_FILE_ONLY TRUE )
                endforeach()
                string(REPLACE ";" "\n" HEADERS_LIST "${HEADERS_LIST}" )            
                math( EXPR index_pack "${index} + ${PACK_IDX}" )
                set ( ${PTYPE}_NAME ${CMAKE_BINARY_DIR}/unity_pack/${PROJECT_NAME}_${index_pack}.${${PTYPE}_PACK_EXP} )
                
                list( APPEND ${PTYPE}_PACK_LIST ${${PTYPE}_NAME} )
                file( WRITE ${${PTYPE}_NAME} ${HEADERS_LIST})
                message( "generated pack     - ${${PTYPE}_NAME}")
                
            endforeach()

            math( EXPR PACK_IDX "${PACK_IDX} + ${${PTYPE}_PACK_SIZE}" )
            set_property( GLOBAL PROPERTY ${PROJECT_NAME}_PACK_IDX "${PACK_IDX}" )
        endforeach() 

        set( ${SOURCE_FILES}  ${${SOURCE_FILES}} ${CPP_PACK_LIST} ${OBJCPP_PACK_LIST} )
        
        list( LENGTH REMAINING_LIST REMAINING_LIST_SIZE )

        message( "       CPP_PACK_SIZE            - ${CPP_PACK_SIZE}")                
        message( "    OBJCPP_PACK_SIZE            - ${OBJCPP_PACK_SIZE}")  
        message( "       CPP_NUMBER_FILES_IN_PACK - ${CPP_NUMBER_FILES_IN_PACK}")
        message( "    OBJCPP_NUMBER_FILES_IN_PACK - ${OBJCPP_NUMBER_FILES_IN_PACK}")
        message( "    IGNORE_LIST_SIZE            - ${IGNORE_LIST_SIZE}")

        message( "    LAST_UNITY_TREE_HASH        - ${LAST_UNITY_TREE_HASH}")
        message( "    UNITY_TREE_HASH             - ${UNITY_TREE_HASH}")

    endif()
endmacro ()