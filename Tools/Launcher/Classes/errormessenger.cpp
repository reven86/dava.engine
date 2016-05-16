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


#include "errormessenger.h"
#include "filemanager.h"
#include <QString>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>

namespace
{
QString errorsMsg[ErrorMessenger::ERROR_COUNT] = {
    "Can't access to documents directory",
    "Network Error",
    "Config parse error",
    "Archive unpacking error",
    "Application is running. Please, close it",
    "Updating error"
};
}

void ErrorMessenger::ShowErrorMessage(ErrorID id, const QString& addInfo)
{
    ShowErrorMessage(id, 0, addInfo);
}

void ErrorMessenger::ShowErrorMessage(ErrorID id, int errorCode, const QString& addInfo)
{
    QString errorMessage = errorsMsg[(int)id];

    if (errorCode)
        errorMessage += QString("\nError Code: %1").arg(errorCode);

    if (!addInfo.isEmpty())
        errorMessage += "\n" + addInfo;

    LogMessage(QtDebugMsg, errorMessage.toStdString().c_str());

    QMessageBox msgBox(QMessageBox::Critical, "Error", errorMessage, QMessageBox::Ok);
    msgBox.exec();
}

int ErrorMessenger::ShowRetryDlg(bool canCancel)
{
    QFlags<QMessageBox::StandardButton> buts = QMessageBox::Retry;
    if (canCancel)
        buts |= QMessageBox::Cancel;

    QMessageBox msgBox(QMessageBox::Critical, "Error", errorsMsg[ERROR_IS_RUNNING], buts);
    msgBox.exec();
    return msgBox.result();
}

void ErrorMessenger::ShowNotificationDlg(const QString& info)
{
    QMessageBox msgBox(QMessageBox::Information, "Launcher", info, QMessageBox::Ok);
    msgBox.exec();
}

void ErrorMessenger::LogMessage(QtMsgType type, const QString& msg)
{
    QString typeStr;
    switch (type)
    {
    case QtDebugMsg:
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

    if (type == QtFatalMsg)
        abort();
}
