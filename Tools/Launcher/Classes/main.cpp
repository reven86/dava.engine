#include "mainwindow.h"
#include "errormessenger.h"

#include "QtHelpers/RunGuard.h"
#include <QApplication>

void FrameworkDidLaunched()
{
}
void FrameworkWillTerminate()
{
}

void LogMessageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    ErrorMessenger::LogMessage(type, msg);
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("DAVA");
    a.setApplicationName("Launcher");
    const QString appUid = "{E5C30634-7624-4D0F-9DD9-C8D52AECA3D0}";
    const QString appUidPath = QCryptographicHash::hash((appUid + QApplication::applicationDirPath()).toUtf8(), QCryptographicHash::Sha1).toHex();
    QtHelpers::RunGuard runGuard(appUidPath);
    if (!runGuard.TryToRun())
        return 0;

    qInstallMessageHandler(LogMessageHandler);

    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    MainWindow w;
    w.show();
    w.setWindowState(Qt::WindowActive);

    return a.exec();
}
