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
    QDir currentDir(".");
    if (!currentDir.exists("platforms"))
    {
        currentDir.mkpath("platforms");
        QFile::copy("qwindows.dll", "platforms/qwindows.dll");
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
