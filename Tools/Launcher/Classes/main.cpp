#include "mainwindow.h"
#include "errormessenger.h"
#include <QApplication>

void LogMessageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    ErrorMessenger::LogMessage(type, msg);
}

int main(int argc, char* argv[])
{
#ifdef Q_OS_WIN
    //this code is deprecated and fix update mechamism from old versions of launcher.
    //remove this block in 2017

    QFileInfo fi(argv[0]);
    QDir currentDir(fi.absoluteDir());
    QString platformsDir = "platforms";
    QString windowsDll = "qwindows.dll";
    currentDir.cd(platformsDir);
    currentDir.removeRecursively();
    currentDir.cdUp();
    //if (!currentDir.exists(platformsDir))
    {
        currentDir.mkpath(platformsDir);
        QFile::copy(windowsDll, platformsDir + "/" + windowsDll);
    }
#endif //windows
    QApplication a(argc, argv);

    qInstallMessageHandler(LogMessageHandler);

    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    MainWindow w;
    w.show();
    w.setWindowState(Qt::WindowActive);

    return a.exec();
}
