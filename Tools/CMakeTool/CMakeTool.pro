TEMPLATE = app

QT += qml quick widgets

CONFIG += c++11

SOURCES += Classes/main.cpp \
    Classes/configstorage.cpp \
    Classes/processwrapper.cpp \
    Classes/filesystemhelper.cpp

RESOURCES += qml.qrc \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    Classes/configstorage.h \
    Classes/processwrapper.h \
    Classes/filesystemhelper.h
