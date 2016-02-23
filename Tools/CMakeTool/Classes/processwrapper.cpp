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


#include "processwrapper.h"
#include <QProgressDialog>
#include <QTimer>
#include <QRegularExpression>

ProcessWrapper::ProcessWrapper(QObject* parent)
    : QObject(parent)
{
    connect(&process, &QProcess::readyReadStandardOutput, this, &ProcessWrapper::OnReadyReadStandardOutput);
    connect(&process, &QProcess::readyReadStandardError, this, &ProcessWrapper::OnReadyReadStandardError);
    connect(&process, &QProcess::stateChanged, this, &ProcessWrapper::OnProcessStateChanged);
    connect(&process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &ProcessWrapper::OnProcessError);
}

ProcessWrapper::~ProcessWrapper()
{
    int startTasksSize = taskQueue.size();
    QTimer performTimer;
    performTimer.setInterval(100);
    performTimer.setSingleShot(false);
    QProgressDialog progressDialog(tr("Finishing tasks: %1").arg(startTasksSize), tr("Cancel"), 0, startTasksSize);
    connect(this, &ProcessWrapper::processOutput, &progressDialog, &QProgressDialog::setLabelText);
    connect(&performTimer, &QTimer::timeout, [&progressDialog, this, startTasksSize]() {
        if (taskQueue.isEmpty() && process.state() == QProcess::NotRunning)
        {
            progressDialog.accept();
        }
        else
        {
            if (progressDialog.wasCanceled())
            {
                taskQueue.clear();
                process.kill();
            }
            progressDialog.setValue(startTasksSize - taskQueue.size());
        }
    });
    if (taskQueue.isEmpty() && process.state() == QProcess::NotRunning)
    {
        return;
    }
    performTimer.start();
    progressDialog.exec();
}

void ProcessWrapper::LaunchCmake(QString command)
{
    taskQueue.enqueue(command);
    if (process.state() == QProcess::NotRunning)
    {
        StartNextCommand();
    }
}

void ProcessWrapper::OnReadyReadStandardOutput()
{
    QString text = process.readAllStandardOutput();
    emit processStandardOutput(text);
    emit processOutput(text);
}

void ProcessWrapper::OnReadyReadStandardError()
{
    QString text = process.readAllStandardError();
    emit processOutput(text);
    emit processStandardError(text);
}

void ProcessWrapper::OnProcessStateChanged(QProcess::ProcessState newState)
{
    QString processState;
    switch (newState)
    {
    case QProcess::NotRunning:
        processState = "not running";
        break;
    case QProcess::Starting:
        processState = "starting";
        break;
    case QProcess::Running:
        processState = "running";
        break;
    }

    emit processStateChanged("cmake process is " + processState);
    if (newState == QProcess::NotRunning)
    {
        QMetaObject::invokeMethod(this, "StartNextCommand", Qt::QueuedConnection);
    }
}

void ProcessWrapper::OnProcessError(QProcess::ProcessError error)
{
    QString processError;
    switch (error)
    {
    case QProcess::FailedToStart:
        processError = "failed to start";
        break;
    case QProcess::Crashed:
        processError = "crashed";
        break;
    case QProcess::Timedout:
        processError = "timed out";
        break;
    case QProcess::ReadError:
        processError = "read error";
        break;
    case QProcess::WriteError:
        processError = "write error";
        break;
    case QProcess::UnknownError:
        processError = "unknown error";
        break;
    }
    emit processErrorChanged("process error: " + processError);
}

void ProcessWrapper::StartNextCommand()
{
    if (taskQueue.isEmpty())
    {
        return;
    }
    QString command = taskQueue.dequeue();
    Q_ASSERT(process.state() == QProcess::NotRunning);
    process.start(command);
    emit processStandardOutput(""); //emit an empty string to make whitespace between build logs
}
