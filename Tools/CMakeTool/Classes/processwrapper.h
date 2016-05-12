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


#ifndef PROCESSWRAPPER_H
#define PROCESSWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QVariant>

class ProcessWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ IsRunning NOTIFY runningChanged)

public:
    explicit ProcessWrapper(QObject* parent = 0);

    Q_INVOKABLE void LaunchCmake(const QString& command, bool needClean, const QString& buildFolder);
    Q_INVOKABLE void FindAndOpenProjectFile(const QString& buildFolder);
    Q_INVOKABLE void OpenFolderInExplorer(const QString& folderPath);
    Q_INVOKABLE void BlockingStopAllTasks();
    Q_INVOKABLE void KillProcess();
    bool IsRunning() const;

signals:
    void processStateChanged(const QString& text);
    void processErrorChanged(const QString& text);
    void processStandardOutput(const QString& text) const;
    void processStandardError(const QString& text) const;
    void testSignal();
    void runningChanged(bool running);

private slots:
    void OnReadyReadStandardOutput();
    void OnReadyReadStandardError();
    void OnProcessStateChanged(QProcess::ProcessState newState);
    void OnProcessError(QProcess::ProcessError error);

private:
    Q_INVOKABLE void StartNextCommand();
    bool CleanBuildFolder(const QString& buildFolder) const;
    void SetRunning(bool running);

    QProcess process;
    struct Task
    {
        Task(const QString& command_, bool needClean_, const QString& buildFolder_)
            : command(command_)
            , needClean(needClean_)
            , buildFolder(buildFolder_)
        {
        }
        QString command;
        bool needClean;
        QString buildFolder;
    };
    QQueue<Task> taskQueue;
    bool running = false;
};

#endif // PROCESSWRAPPER_H
