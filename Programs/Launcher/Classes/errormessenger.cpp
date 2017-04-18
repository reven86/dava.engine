#include "errormessenger.h"
#include "filemanager.h"
#include <QString>
#include <QMessageBox>
#include <QDateTime>
#include <QApplication>
#include <QFile>

namespace ErrorMessenger
{
QString errorsMsg[ERROR_COUNT] = {
    "Can't access to documents directory",
    "Network Error",
    "Config parse error",
    "Archive unpacking error",
    "Application %1 is running. Please, close it.",
    "Updating error",
    "Can not find path",
    "File error"
};

void ShowErrorMessage(ErrorID id, const QString& addInfo)
{
    ShowErrorMessage(id, 0, addInfo);
}

void ShowErrorMessage(ErrorID id, int errorCode, const QString& addInfo)
{
    QString title = errorsMsg[(int)id];
    QString errorMessage;
    if (errorCode)
        errorMessage += QString("\nError Code: %1").arg(errorCode);

    if (!addInfo.isEmpty())
        errorMessage += "\n" + addInfo;

    LogMessage(QtDebugMsg, errorMessage.toStdString().c_str());

    QMessageBox msgBox(QMessageBox::Critical, title, errorMessage, QMessageBox::Ok, qApp->activeWindow());
    msgBox.exec();
}

int ShowRetryDlg(const QString& appName, const QString& appPath, bool canCancel)
{
    QFlags<QMessageBox::StandardButton> buts = QMessageBox::Retry;
    if (canCancel)
        buts |= QMessageBox::Cancel;

    QString message = errorsMsg[ERROR_IS_RUNNING].arg(appName);

    if (!appPath.isEmpty())
    {
        message += "\n" + appPath;
    }
    QMessageBox msgBox(QMessageBox::Critical, "Error", message, buts);
    msgBox.exec();
    return msgBox.result();
}

void ShowNotificationDlg(const QString& info)
{
    QMessageBox msgBox(QMessageBox::Information, "Launcher", info, QMessageBox::Ok);
    msgBox.exec();
}

void LogMessage(QtMsgType type, const QString& msg)
{
    QString typeStr;
    switch (type)
    {
    case QtDebugMsg:
    case QtInfoMsg:
        typeStr = "DEBUG";
        break;
    case QtWarningMsg:
        typeStr = "WARNING";
        break;
    case QtCriticalMsg:
        typeStr = "CRITICAL";
        break;
    case QtFatalMsg:
        typeStr = "FATAL";
        break;
    }
    QFile logFile;
    QString logPath = FileManager::GetDocumentsDirectory() + "launcher.log";
    logFile.setFileName(logPath);
    QString time = QDateTime::currentDateTime().toString("[dd.MM.yyyy - hh:mm:ss]");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        logFile.write((QString("%1 (%2): %3\n").arg(time).arg(typeStr).arg(msg)).toStdString().c_str());
        logFile.flush();
        logFile.close();
    }
}
}
