#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T21:40:18
#
#-------------------------------------------------

DEFINES += _SCL_SECURE_NO_WARNINGS

QT       += core gui network webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Launcher
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    filemanager.cpp \
    configparser.cpp \
    applicationmanager.cpp \
    updatedialog.cpp \
    buttonswidget.cpp \
    processhelper.cpp \
    zipunpacker.cpp \
    errormessanger.cpp \
    selfupdater.cpp \
    configdownloader.cpp

HEADERS  += mainwindow.h \
    filemanager.h \
    configparser.h \
    applicationmanager.h \
    updatedialog.h \
    buttonswidget.h \
    defines.h \
    processhelper.h \
    zipunpacker.h \
    errormessanger.h \
    selfupdater.h \
    configdownloader.h

FORMS    += mainwindow.ui \
    updatedialog.ui \
    buttonswidget.ui \
    selfupdater.ui \
    configdownloader.ui

RESOURCES += \
    icons.qrc

win32: RC_FILE = Launcher.rc
macx: ICON = icon.icns
macx: QMAKE_INFO_PLIST = Info.plist

INCLUDEPATH += $$PWD/yaml-cpp/include
DEPENDPATH += $$PWD/yaml-cpp/include

INCLUDEPATH += $$PWD/quazip
DEPENDPATH += $$PWD/quazip

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/yaml-cpp/libs/ -llibyaml-cppmd -luser32 -lshell32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/yaml-cpp/libs/ -llibyaml-cppmdd -luser32 -lshell32
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/yaml-cpp/libs/libyaml-cppmd.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/yaml-cpp/libs/libyaml-cppmdd.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/quazip/lib/ -lzlib -lquazip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/quazip/lib/ -lzlibd -lquazipd
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/quazip/lib/zlib.lib $$PWD/quazip/lib/quazip.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/quazip/lib/zlibd.lib $$PWD/quazip/lib/quazipd.lib

macx: LIBS += -L$$PWD/yaml-cpp/libs/ -lyaml-cpp_osx
macx: PRE_TARGETDEPS += $$PWD/yaml-cpp/libs/libyaml-cpp_osx.a

macx: LIBS += -F//System/Library/Frameworks/ -framework ApplicationServices
macx: INCLUDEPATH += /System/Library/Frameworks/ApplicationServices.framework/Frameworks/HIServices.framework/Headers/
macx: DEPENDPATH += /System/Library/Frameworks/ApplicationServices.framework/Frameworks/HIServices.framework/Headers/
macx: LIBS += -lz
macx: HEADERS += \
    quazip/quazip/JlCompress.h \
    quazip/quazip/zip.h \
    quazip/quazip/unzip.h \
    quazip/quazip/quazipnewinfo.h \
    quazip/quazip/quazipfileinfo.h \
    quazip/quazip/quazipfile.h \
    quazip/quazip/quazipdir.h \
    quazip/quazip/quazip.h \
    quazip/quazip/quazip_global.h \
    quazip/quazip/quaziodevice.h \
    quazip/quazip/quagzipfile.h \
    quazip/quazip/quacrc32.h \
    quazip/quazip/quachecksum32.h \
    quazip/quazip/quaadler32.h \
    quazip/quazip/ioapi.h
macx: SOURCES += \
    quazip/quazip/quazipfile.cpp \
    quazip/quazip/quazip.cpp \
    quazip/quazip/qioapi.cpp \
    quazip/quazip/JlCompress.cpp \
    quazip/quazip/zip.c \
    quazip/quazip/unzip.c \
    quazip/quazip/quazipfileinfo.cpp \
    quazip/quazip/quazipnewinfo.cpp \
    quazip/quazip/quazipdir.cpp \
    quazip/quazip/quaziodevice.cpp \
    quazip/quazip/quagzipfile.cpp \
    quazip/quazip/quacrc32.cpp \
    quazip/quazip/quaadler32.cpp
