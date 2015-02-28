include ( GlobalVariables )

find_program( ANT_COMMAND NAMES ant ${ANDROID_ANT}/ant ${ANDROID_ANT}/bin/ant  
                                ant.bat ${ANDROID_ANT}/ant.bat ${ANDROID_ANT}/bin/ant.bat  )

if( NOT CMAKE_EXTRA_GENERATOR AND NOT ANT_COMMAND )
    message( "Error !!!: Please set the correct path to ANDROID_ANT in file DavaConfig.in"  )
    message( "" )
    exit()

endif()


find_program( ANDROID_COMMAND NAMES android ${ANDROID_SDK}/android ${ANDROID_SDK}/tools/android  
                                    android.bat ${ANDROID_SDK}/android.bat ${ANDROID_SDK}/tools/android.bat  )
if( NOT ANDROID_COMMAND )
    message( "Error !!!: Please set the correct path to ANDROID_SDK in file DavaConfig.in"  )
    message( " " )
    exit()

endif()

message( ANT_COMMAND     " - ${ANT_COMMAND}" )
message( ANDROID_COMMAND " - ${ANDROID_COMMAND}" )