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
