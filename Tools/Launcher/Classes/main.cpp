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


#include "mainwindow.h"
#include "filemanager.h"
#include "errormessanger.h"
#include <QApplication>

void LogMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    ErrorMessanger::Instance()->LogMessage(type, msg);
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    char **argv1 = new char *[argc + 2];
    memcpy(argv1, argv, argc * sizeof(char *));

    argv1[argc] = "-platformpluginpath";
    argv1[argc + 1] = ".";

    argc += 2;
    argv = argv1;
#endif // Q_OS_WIN

    QApplication a(argc, argv);

#ifdef Q_OS_WIN
    delete[] argv1;
#endif // Q_OS_WIN

    ErrorMessanger::Instance();
    qInstallMessageHandler(LogMessageHandler);

    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    MainWindow w;
    w.show();
    w.setWindowState(Qt::WindowActive);

    return a.exec();
}
