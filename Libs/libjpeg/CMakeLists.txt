CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(libjpeg C)

SET(libjpeg_SRCS jmemnobs.c jaricom.c jcapimin.c jcapistd.c jcarith.c jccoefct.c jccolor.c
    jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c
    jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c jdarith.c jdatadst.c jdatasrc.c
    jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c
    jdmerge.c jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c jfdctint.c
    jidctflt.c jidctfst.c jidctint.c jquant1.c jquant2.c jutils.c jmemmgr.c)
SET(libjpeg_HDRS cderror.h cdjpeg.h jdct.h jinclude.h jmemsys.h jpegint.h
    jversion.h transupp.h jerror.h jmorecfg.h jpeglib.h)

IF(MSVC)
    ADD_DEFINITIONS(/D_UNICODE /DUNICODE)
    LIST(APPEND libjpeg_SRCS win32/jpeg.def)
ENDIF(MSVC)
ADD_DEFINITIONS(-DTEMP_DIRECTORY="/sdcard/")
ADD_LIBRARY(jpeg ${libjpeg_SRCS} ${libjpeg_HDRS})

INSTALL(TARGETS jpeg
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

INSTALL(FILES ${libjpeg_HDRS} DESTINATION include/jpeg)