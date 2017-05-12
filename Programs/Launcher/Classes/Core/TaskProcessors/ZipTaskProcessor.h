#pragma once

#include "Core/TaskProcessors/BaseTaskProcessor.h"

#include <QProcess>

class UnzipTask;

class ZipTaskProcessor final : public QObject, public BaseTaskProcessor
{
    Q_OBJECT

public:
    ~ZipTaskProcessor();

private slots:
    void OnErrorOccurred(QProcess::ProcessError error);
    void OnFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void OnReadyReadStandardOutput();

private:
    enum eState
    {
        LIST_ARCHIVE,
        UNPACK
    };

    void AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier) override;
    void Terminate() override;

    void ApplyState();

    void ProcessOutputForList(const QByteArray& line);
    void ProcessOutputForUnpack(const QByteArray& line);

    QString GetArchiverPath() const;

    struct TaskParams
    {
        TaskParams(std::unique_ptr<BaseTask>&& task, const ReceiverNotifier& notifier);
        ~TaskParams();

        QProcess process;
        std::unique_ptr<UnzipTask> task;

        ReceiverNotifier notifier;
        eState state = LIST_ARCHIVE;
        bool foundOutputData = false;
        std::map<QString, quint64> filesAndSizes;
        quint64 matchedSize = 0;
        quint64 totalSize = 0;
    };
    std::unique_ptr<TaskParams> currentTaskParams;
};
