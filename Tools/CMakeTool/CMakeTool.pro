TEMPLATE = app

QT += qml quick widgets
CONFIG += C++11

SOURCES += Classes/main.cpp \
    Classes/configstorage.cpp \
    Classes/filesystemhelper.cpp \
    Classes/processwrapper.cpp

RESOURCES += DataQt/resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    Classes/configstorage.h \
    Classes/filesystemhelper.h \
    Classes/processwrapper.h

