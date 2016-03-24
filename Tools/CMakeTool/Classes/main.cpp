/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


#include <QApplication>
#include <QString>
#include <QQmlApplicationEngine>
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QtQuick>
#include <QtQML>
#include "configstorage.h"
#include "processwrapper.h"
#include "filesystemhelper.h"
#include "help.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("DAVA");
    app.setApplicationName("CMakeTool");
    QQmlApplicationEngine engine;
    auto rootContext = engine.rootContext();
    qmlRegisterType<ProcessWrapper>("Cpp.Utils", 1, 0, "ProcessWrapper");
    qmlRegisterType<FileSystemHelper>("Cpp.Utils", 1, 0, "FileSystemHelper");
    qmlRegisterType<ConfigStorage>("Cpp.Utils", 1, 0, "ConfigStorage");
    qmlRegisterType<Help>("Cpp.Utils", 1, 0, "Help");

    rootContext->setContextProperty("applicationDirPath", app.applicationDirPath());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().empty())
    {
        QMessageBox::critical(nullptr, QObject::tr("Failed to load QML file"), QObject::tr("QML file not loaded!"));
        return 0;
    }
    return app.exec();
}
