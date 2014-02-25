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

SOURCES += \
        Classes/main.cpp \
        Classes/FrameworkMain.cpp \
        Classes/GameCore.cpp \
        Classes/UI/mainwindow.cpp \
        Classes/UI/TestScreen.cpp \
        Classes/UI/davaglwidget.cpp \
        Classes/Metadata/UIControlMetadata.cpp \
    Classes/UI/hierarchytreewidget.cpp \
    Classes/UI/librarywidget.cpp \
    Classes/UI/controllist.cpp \
    UI/basepropertygridwidget.cpp \
    UI/rectpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/basepropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/rectpropertygridwidget.cpp \
    UI/propertygridcontainerwidget.cpp \
    Classes/UI/PropertyGridWidgets/propertygridcontainerwidget.cpp \
    Classes/Metadata/UIButtonMetadata.cpp \
    Classes/Metadata/BaseMetadata.cpp \
    Classes/Metadata/MetadataFactory.cpp \
    UI/controlpropertygridwidget.cpp \
    UI/flagspropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/flagspropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/controlpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/screenpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/platformpropertygridwidget.cpp \
    Classes/Metadata/UITextControlMetadata.cpp \
    Classes/UI/PropertyGridWidgets/textpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/backgroundpropertygridwidget.cpp \
	Classes/UI/fontmanagerdialog.cpp \
    Classes/UI/createplatformdlg.cpp \
	Classes/UI/createscreendlg.cpp \
    Classes/UI/PropertyGridWidgets/backgroundpropertygridwidget.cpp \
	Classes/UI/PropertyGridWidgets/textpropertygridwidget.cpp \
    Classes/UI/librarywidget.cpp \
    Classes/UI/PropertyGridWidgets/statepropertygridwidget.cpp \
    Classes/UI/hierarchytreecontrol.cpp \
    Classes/UI/Dialogs/localizationeditordialog.cpp \
    Classes/UI/QColorButton.cpp \
    Classes/UI/StateComboBoxItemDelegate.cpp \
    Classes/Metadata/UITextFieldMetadata.cpp \
    UI/PropertyGridWidgets/uitextfieldpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/uitextfieldpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/sliderpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/alignspropertygridwidget.cpp \
    UI/spinnerpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/spinnerpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/listpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/scrollviewpropertygridwidget.cpp \
    Classes/UI/PropertyGridWidgets/previewsettingsdialog.cpp \
    Classes/UI/PropertyGridWidgets/editeditorsettingsdialog.cpp
    Classes/UI/particleeffectpropertygridwidget.cpp

HEADERS  += \
        Classes/AppScreens.h \
        Classes/FrameworkMain.h \
        Classes/GameCore.h \
        Classes/UI/mainwindow.h  \
        Classes/UI/TestScreen.h  \
        Classes/UI/davaglwidget.h \
        Classes/Metadata/UIControlMetadata.h \
    Classes/UI/hierarchytreewidget.h \
    Classes/UI/librarywidget.h \
    Classes/UI/controllist.h \
    UI/basepropertygridwidget.h \
    UI/rectpropertygridwidget.h \
    UI/basepropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/basepropertygridwidget.h \
    UI/rectpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/rectpropertygridwidget.h \
    UI/propertygridcontainerwidget.h \
    Classes/UI/PropertyGridWidgets/propertygridcontainerwidget.h \
    Classes/Metadata/UIButtonMetadata.h \
    Classes/Metadata/BaseMetadata.h \
    Classes/Metadata/MetadataFactory.h \
    UI/controlpropertygridwidget.h \
    UI/flagspropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/flagspropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/controlpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/platformpropertygridwidget.h \
    Classes/Metadata/UITextControlMetadata.h \
    Classes/UI/PropertyGridWidgets/textpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/backgroundpropertygridwidget.h \
	Classes/UI/fontmanagerdialog.h \
    Classes/UI/createplatformdlg.h \
	Classes/UI/createscreendlg.h \
    Classes/UI/PropertyGridWidgets/backgroundpropertygridwidget.h \
	Classes/UI/PropertyGridWidgets/textpropertygridwidget.h \
    Classes/UI/librarywidget.h \
    UI/statepropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/statepropertygridwidget.h \
    Classes/UI/hierarchytreecontrol.h \
    Classes/UI/Dialogs/localizationeditordialog.h \
    Classes/UI/QColorButton.h \
    Classes/UI/StateComboBoxItemDelegate.h \
    Classes/Metadata/UITextFieldMetadata.h \
    UI/PropertyGridWidgets/uitextfieldpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/sliderpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/alignspropertygridwidget.h \
    UI/spinnerpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/spinnerpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/listpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/scrollviewpropertygridwidget.h \
    Classes/UI/PropertyGridWidgets/previewsettingsdialog.h \
    Classes/UI/PropertyGridWidgets/editeditorsettingsdialog.h
    Classes/UI/particleeffectpropertygridwidget.h

FORMS    += \
        UI/mainwindow.ui \
        UI/davaglwidget.ui \
    UI/hierarchytreewidget.ui \
    UI/basepropertygridwidget.ui \
    UI/rectpropertygridwidget.ui \
	UI/propertygridcontainerwidget.ui \
    UI/librarywidget.ui \
    UI/controlpropertygridwidget.ui \
    UI/flagspropertygridwidget.ui \
    UI/platformpropertygridwidget.ui \
    UI/screenpropertygridwidget.ui \
    UI/textpropertygridwidget.ui \
    UI/backgroundpropertygridwidget.ui \
	UI/fontmanagerdialog.ui \
	UI/createscreendlg.ui \
    UI/textpropertygridwidget.ui \
    UI/backgroundpropertygridwidget.ui \
    UI/Dialogs/fontmanagerdialog.ui \
    UI/Dialogs/createscreendlg.ui \
    UI/Dialogs/createplatformdlg.ui \
    UI/PropertyGridWidgets/textpropertygridwidget.ui \
    UI/PropertyGridWidgets/statepropertygridwidget.ui \
    UI/PropertyGridWidgets/screenpropertygridwidget.ui \
    UI/PropertyGridWidgets/rectpropertygridwidget.ui \
    UI/PropertyGridWidgets/propertygridcontainerwidget.ui \
    UI/PropertyGridWidgets/platformpropertygridwidget.ui \
    UI/PropertyGridWidgets/flagspropertygridwidget.ui \
    UI/PropertyGridWidgets/controlpropertygridwidget.ui \
    UI/PropertyGridWidgets/basepropertygridwidget.ui \
    UI/PropertyGridWidgets/backgroundpropertygridwidget.ui \
    UI/Dialogs/localizationeditordialog.ui \
    UI/PropertyGridWidgets/uitextfieldpropertygridwidget.ui \
    UI/PropertyGridWidgets/sliderpropertygridwidget.ui \
    UI/PropertyGridWidgets/alignspropertygridwidget.ui \
    UI/statepropertygridwidget.ui \
    UI/spinnerpropertygridwidget.ui \
    UI/listpropertygridwidget.ui \
    UI/alignspropertygridwidget.ui \
    UI/sliderpropertygridwidget.ui \
    UI/scrollviewpropertygridwidget.ui \
    UI/scrollcontrolpropertygridwidget.ui \
    UI/previewsettingsdialog.ui \
    UI/editeditorsettingsdialog.ui
    UI/particleeffectpropertygridwidget.ui


#debug dependent
CONFIG(debug, debug|release): {
DEFINES += DAVA_DEBUG
}


macx {

QMAKE_INFO_PLIST = Info.plist

RESOURCES     += Data/icons.qrc

FILETYPES.files = Data/
FILETYPES.path = Contents/Resources
QMAKE_BUNDLE_DATA += FILETYPES

CONFIG += x86 x86_64

QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.7.sdk

LIBS += -framework Cocoa -framework OpenAL -lz

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
