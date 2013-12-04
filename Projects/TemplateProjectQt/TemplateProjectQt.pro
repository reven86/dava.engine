#-------------------------------------------------
#
# Project created by QtCreator 2012-06-29T16:59:23
#
#-------------------------------------------------

QT       += core gui opengl network

TARGET = TemplateProjectQt
TEMPLATE = app


INCLUDEPATH += ../dava.framework/Sources/Internal
INCLUDEPATH += ../dava.framework/Sources/External
INCLUDEPATH += ../dava.framework/Sources/External/Freetype
INCLUDEPATH += ../dava.framework/Libs/oggvorbis/include
INCLUDEPATH += ../dava.framework/Sources/Libs/include
INCLUDEPATH += Classes

SOURCES += main.cpp \
        mainwindow.cpp \
        Classes/FrameworkMain.cpp \
        Classes/GameCore.cpp \
        Classes/TestScreen.cpp \
        Classes/davaglwidget.cpp

HEADERS  += mainwindow.h  \
        Classes/AppScreens.h  \
        Classes/FrameworkMain.h  \
        Classes/GameCore.h  \
        Classes/TestScreen.h  \
        Classes/davaglwidget.h

FORMS    += mainwindow.ui \
        Classes/davaglwidget.ui


#debug dependent
CONFIG(debug, debug|release): {
DEFINES += DAVA_DEBUG
}


macx {

QMAKE_INFO_PLIST = Info.plist


FILETYPES.files = Data/
FILETYPES.path = Contents/Resources
QMAKE_BUNDLE_DATA += FILETYPES

CONFIG += x86 x86_64

QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.7.sdk

LIBS += -framework Cocoa -framework -lz

LIBS += -L../dava.framework/Libs/libs/ -lmongodb_macos
LIBS += -L../dava.framework/Libs/libs/ -lpng_macos
LIBS += -L../dava.framework/Libs/libs/ -lxml_macos
LIBS += -L../dava.framework/Libs/libs/ -lyaml_macos
LIBS += -L../dava.framework/Libs/libs/ -lcurl_ios_mac
LIBS += -L../dava.framework/Libs/libs/ -ltheora_macos
LIBS += -L../dava.framework/Libs/freetype/lib/ -lfreetype_macos
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -lvorbisfile_macos
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -lvorbis_macos
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -logg_macos

INCLUDEPATH += ../dava.framework/Libs/libs
INCLUDEPATH += ../dava.framework/Libs/freetype/include

PRE_TARGETDEPS += ../dava.framework/Libs/libs/libmongodb_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/libs/libpng_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/libs/libxml_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/libs/libyaml_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/libs/libcurl_ios_mac.a
PRE_TARGETDEPS += ../dava.framework/Libs/libs/libtheora_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/freetype/lib/libfreetype_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/oggvorbis/lib/libvorbisfile_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/oggvorbis/lib/libvorbis_macos.a
PRE_TARGETDEPS += ../dava.framework/Libs/oggvorbis/lib/libogg_macos.a

DEFINES += DDARWIN_NO_CARBON FT2_BUILD_LIBRARY

}

win32{

APP_DATA_FILES.files = Data/
APP_DATA_FILES.path = Data
QMAKE_BUNDLE_DATA += APP_DATA_FILES


APP_DATA_FILES.files = glew32.dll
APP_DATA_FILES.path = glew32.dll
QMAKE_BUNDLE_DATA += APP_DATA_FILES


INCLUDEPATH += ../dava.framework/Libs/glew/include
INCLUDEPATH += $$quote("c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/include")
INCLUDEPATH += $$quote("C:/Program Files/OpenAL 1.1 SDK/include")
INCLUDEPATH += $$quote("C:/Program Files (x86)/OpenAL 1.1 SDK/include")

CONFIG(release, debug|release) {
LIBS += -L../dava.framework/Libs/libs/ -llibmongodb_win
LIBS += -L../dava.framework/Libs/libs/ -lpnglib_win
LIBS += -L../dava.framework/Libs/libs/ -llibxml_win
LIBS += -L../dava.framework/Libs/libs/ -llibyaml_win
LIBS += -L../dava.framework/Libs/libs/ -lzlib
LIBS += -L../dava.framework/Libs/freetype/lib/ -lfreetype246MT
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -llibogg_static
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -llibvorbis_static
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -llibvorbisfile_static

}
CONFIG(debug, debug|release) {
LIBS += -L../dava.framework/Libs/libs/ -llibmongodb_wind
LIBS += -L../dava.framework/Libs/libs/ -lpnglib_wind
LIBS += -L../dava.framework/Libs/libs/ -llibxml_wind
LIBS += -L../dava.framework/Libs/libs/ -llibyaml_wind
LIBS += -L../dava.framework/Libs/libs/ -lzlibd
LIBS += -L../dava.framework/Libs/freetype/lib/ -lfreetype246MT_D
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -llibogg_static_d
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -llibvorbis_static_d
LIBS += -L../dava.framework/Libs/oggvorbis/lib/ -llibvorbisfile_static_d

QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd
}

LIBS += -L../dava.framework/Libs/libs/ -llibtheora_win
LIBS += -L../dava.framework/Libs/glew/lib/ -lglew32

LIBS += -L$$quote("c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/") -lDxErr
LIBS += -L$$quote("c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/") -ld3d9
LIBS += -lshfolder
LIBS += -lShell32
LIBS += -lopengl32
LIBS += -L$$quote("C:/Program Files (x86)/OpenAL 1.1 SDK/libs/win32") -lOpenAL32

}


DEPENDPATH += INCLUDEPATH
