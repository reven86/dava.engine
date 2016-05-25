#include "mainwindow.h"
#include "filemanager.h"
#include "errormessanger.h"
#include <QApplication>

void LogMessageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    ErrorMessanger::Instance()->LogMessage(type, msg);
}

int main(int argc, char* argv[])
{
#ifdef Q_OS_WIN
    char** argv1 = new char*[argc + 2];
    memcpy(argv1, argv, argc * sizeof(char*));

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
