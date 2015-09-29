include ( GlobalVariables )

find_program( ANT_COMMAND NAMES ant /opt/local/bin/ant ${ANDROID_ANT}/ant ${ANDROID_ANT}/bin/ant /usr/local/bin/ant
                                ant.bat ${ANDROID_ANT}/ant.bat ${ANDROID_ANT}/bin/ant.bat )

if( NOT ANT_COMMAND)
    message( FATAL_ERROR "Can't find ANT tool search in: ant /opt/local/bin/ant ${ANDROID_ANT}/ant ${ANDROID_ANT}/bin/ant \
    ant.bat ${ANDROID_ANT}/ant.bat ${ANDROID_ANT}/bin/ant.bat" )
endif()

if( NOT CMAKE_EXTRA_GENERATOR AND NOT ANT_COMMAND )
    message( FATAL_ERROR "Please set the correct path to ANDROID_ANT in file DavaConfig.in"  )
endif()

find_program( ANDROID_COMMAND NAMES android ${ANDROID_SDK}/android ${ANDROID_SDK}/tools/android  
                                    android.bat ${ANDROID_SDK}/android.bat ${ANDROID_SDK}/tools/android.bat  )
if( NOT ANDROID_COMMAND )
    message( FATAL_ERROR "Please set the correct path to ANDROID_SDK in file DavaConfig.in"  )
endif()

find_program( JAVACOMPILER_COMMAND NAMES javac $ENV{JAVA_HOME}/bin/javac /usr/bin/javac )

if ( NOT JAVACOMPILER_COMMAND )
    message( FATAL_ERROR "Can't find javac tool, please point JAVA_HOME to JDK directory" )
endif()

message( ANT_COMMAND     " - ${ANT_COMMAND}" )
message( ANDROID_COMMAND " - ${ANDROID_COMMAND}" )