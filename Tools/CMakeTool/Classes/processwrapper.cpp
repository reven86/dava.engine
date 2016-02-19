#include "processwrapper.h"
#include <QProgressDialog>
#include <QTimer>

ProcessWrapper::ProcessWrapper(QObject *parent) : QObject(parent)
{
    connect(&process, &QProcess::readyReadStandardOutput, this, &ProcessWrapper::OnReadyReadStandardOutput);
    connect(&process, &QProcess::readyReadStandardError, this, &ProcessWrapper::OnReadyReadStandardError);
    connect(&process, &QProcess::stateChanged, this, &ProcessWrapper::OnProcessStateChanged);
    connect(&process, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &ProcessWrapper::OnProcessError);
}

ProcessWrapper::~ProcessWrapper()
{
    int startTasksSize = taskQueue.size();
    QTimer performTimer;
    performTimer.setInterval(100);
    performTimer.setSingleShot(false);
    QProgressDialog progressDialog(tr("Finishing tasks: %1").arg(startTasksSize), tr("Cancel"), 0, startTasksSize);
    connect(this, &ProcessWrapper::processOutput, &progressDialog, &QProgressDialog::setLabelText);
    connect(&performTimer, &QTimer::timeout, [&progressDialog, this, startTasksSize](){
        if(taskQueue.isEmpty() && process.state() == QProcess::NotRunning)
        {
            progressDialog.accept();
        }
        else
        {
            if(progressDialog.wasCanceled())
            {
                taskQueue.clear();
                process.kill();
            }
            progressDialog.setValue(startTasksSize - taskQueue.size());
        }
    });
    if(taskQueue.isEmpty() && process.state() == QProcess::NotRunning)
    {
        return;
    }
    performTimer.start();
    progressDialog.exec();

}

void ProcessWrapper::LaunchCmake(QString command)
{
    taskQueue.enqueue(command);
    if(process.state() == QProcess::NotRunning)
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
    emit processStandardError(text);
    emit processOutput(text);
}

void ProcessWrapper::OnProcessStateChanged(QProcess::ProcessState newState)
{
    QString processState;
    switch(newState)
    {
    case QProcess::NotRunning: processState = "not running"; break;
    case QProcess::Starting: processState = "starting"; break;
    case QProcess::Running: processState = "running"; break;
    }

    emit processStateChanged("cmake process is " + processState);
    if(newState == QProcess::NotRunning)
    {
        QMetaObject::invokeMethod(this, "StartNextCommand", Qt::QueuedConnection);
    }
}

void ProcessWrapper::OnProcessError(QProcess::ProcessError error)
{
    QString processError;
    switch(error)
    {
    case QProcess::FailedToStart: processError = "failed to start"; break;
    case QProcess::Crashed: processError = "crashed"; break;
    case QProcess::Timedout: processError = "timed out"; break;
    case QProcess::ReadError: processError = "read error"; break;
    case QProcess::WriteError: processError = "write error"; break;
    case QProcess::UnknownError: processError = "unknown error"; break;
    }
    emit processErrorChanged("process error: " + processError);
}

void ProcessWrapper::StartNextCommand()
{
    if(taskQueue.isEmpty())
    {
        return;
    }
    QString command = taskQueue.dequeue();
    Q_ASSERT(process.state() == QProcess::NotRunning);
    process.start(command);
}
