
if( ANDROID )
  set( ERASE_FILES )

else()
  set( ERASE_FILES 	LocalizationAndriod.cpp 
  					LocalizationAndroid.cpp )

endif()

define_source_files (PARENT_SCOPE GLOB_ERASE_FILES ${ERASE_FILES} )

