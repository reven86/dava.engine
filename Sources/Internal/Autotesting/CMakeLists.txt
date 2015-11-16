if( NOT DAVA_DISABLE_AUTOTESTS )
    set( ADDED_CPP_PATTERNS *.cpp *.c )  
    set( ADDED_H_PATTERNS   *.h   )

    define_source_files( PARENT_SCOPE 
      GLOB_CPP_PATTERNS    ${ADDED_CPP_PATTERNS}
      GLOB_H_PATTERNS      ${ADDED_H_PATTERNS}
      GLOB_ERASE_FILES     ${ERASE_FILES} 
    )

endif()

