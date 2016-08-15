macro( copy_at_post_build SRC DST )

	add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC} ${DST})

endmacro()

#[[if ( DATA_FOUND )
    return ()
endif ()
set ( DATA_FOUND 1 )]]

#get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )
#add_subdirectory ( "${CMAKE_CURRENT_LIST_DIR}/../../../Tools/Data" ${CMAKE_CURRENT_BINARY_DIR}/Data )

#set ( TOOL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
#set ( DATA_DIR "${CURRENT_DIR}/../../../Tools/Data" )

#add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD
       #COMMAND ${CMAKE_COMMAND} -E copy ${DATA_DIR}/libfmodevent.dylib ${TOOL_OUTPUT_DIR}
       #COMMAND ${CMAKE_COMMAND} -E copy ${DATA_DIR}/libfmodex.dylib ${TOOL_OUTPUT_DIR}
       #COMMAND ${CMAKE_COMMAND} -E copy ${DATA_DIR}/libTextureConverter.dylib ${TOOL_OUTPUT_DIR}
#       COMMAND ${CMAKE_COMMAND} -E copy_directory ${DATA_DIR} ${TOOL_OUTPUT_DIR}/Data)