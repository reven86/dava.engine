#include "Core/TaskProcessors/ZipTaskProcessor.h"
#include "Core/Tasks/UnzipTask.h"

#include <QProcess>
#include <QApplication>
#include <QDir>
#include <QRegularExpression>

ZipTaskProcessor::TaskParams::TaskParams(std::unique_ptr<BaseTask>&& task_, const ReceiverNotifier& notifier_)
    : task(static_cast<UnzipTask*>(task_.release()))
    , notifier(notifier_)
{
    notifier.NotifyStarted(task.get());
}

ZipTaskProcessor::TaskParams::~TaskParams()
{
    if (process.state() != QProcess::NotRunning)
    {
        process.terminate();
    }
    notifier.NotifyFinished(task.get());
}

void ZipTaskProcessor::AddTask(std::unique_ptr<BaseTask>&& task, ReceiverNotifier notifier)
{
    Q_ASSERT(task->GetTaskType() == BaseTask::ZIP_TASK);
    currentTaskParams = std::make_unique<TaskParams>(std::move(task), notifier);
    CheckProgramState();
    if (currentTaskParams->task->GetError().isEmpty() == false)
    {
        return;
    }

    connect(&currentTaskParams->process, &QProcess::readyReadStandardOutput, this, &ZipTaskProcessor::OnReadyReadStandardOutput);
    connect(&currentTaskParams->process, &QProcess::errorOccurred, this, &ZipTaskProcessor::OnErrorOccurred);
    connect(&currentTaskParams->process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &ZipTaskProcessor::OnFinished);

    ApplyState();
}

void ZipTaskProcessor::Terminate()
{
    currentTaskParams = nullptr;
}

void ZipTaskProcessor::OnErrorOccurred(QProcess::ProcessError error)
{
    switch (error)
    {
    case QProcess::FailedToStart:
        currentTaskParams->task->SetError(QObject::tr("Failed to launch archiver"));
        break;
    case QProcess::Crashed:
        currentTaskParams->task->SetError(QObject::tr("Archiver crashed"));
        break;
    case QProcess::Timedout:
        currentTaskParams->task->SetError(QObject::tr("Archiver time out error"));
        break;
    case QProcess::ReadError:
        currentTaskParams->task->SetError(QObject::tr("Failed to read output from archiver"));
        break;
    case QProcess::WriteError:
        currentTaskParams->task->SetError(QObject::tr("Failed to write to archiver"));
        break;
    case QProcess::UnknownError:
        currentTaskParams->task->SetError(QObject::tr("Archiver crashed with an unknown error"));
        break;
    }
}

void ZipTaskProcessor::OnFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus != QProcess::NormalExit)
    {
        currentTaskParams->task->SetError(QObject::tr("Archiver reports about error in current archive"));
    }
    else
    {
        if (currentTaskParams->state == LIST_ARCHIVE)
        {
            const QList<quint64>& values = currentTaskParams->filesAndSizes.values();
            currentTaskParams->totalSize = std::accumulate(values.begin(), values.end(), 0);
            currentTaskParams->state = UNPACK;
            ApplyState();
        }
        else
        {
            Terminate();
        }
    }
}

void ZipTaskProcessor::OnReadyReadStandardOutput()
{
    while (currentTaskParams != nullptr && currentTaskParams->process.canReadLine())
    {
        QByteArray line = currentTaskParams->process.readLine();
        if (currentTaskParams->state == LIST_ARCHIVE)
        {
            ProcessOutputForList(line);
        }
        else
        {
            ProcessOutputForUnpack(line);
        }
    }
    qApp->processEvents();
}

void ZipTaskProcessor::ApplyState()
{
    QString processAddr = GetArchiverPath();
    QStringList arguments;
    if (currentTaskParams->state == LIST_ARCHIVE)
    {
        arguments << "l" << currentTaskParams->task->GetArchivePath();
    }
    else
    {
        //this is needed for correct work with pathes contains whitespace
        QString nativeOutPath = QDir::toNativeSeparators(currentTaskParams->task->GetOutputPath());
        arguments << "x"
                  << "-y"
                  << "-bb1"
                  << currentTaskParams->task->GetArchivePath()
                  << "-o" + nativeOutPath;
    }
    currentTaskParams->process.start(processAddr, arguments);
}

void ZipTaskProcessor::ProcessOutputForList(const QByteArray& line)
{
    if (line.startsWith("----------")) //this string occurrs two times: before file list and at the and of file list
    {
        currentTaskParams->foundOutputData = !currentTaskParams->foundOutputData;
        return;
    }
    if (currentTaskParams->foundOutputData == false)
    {
        return;
    }
    QString str(line);

    const int SIZE_INDEX = 26; //fixed index for size
    const int SIZE_STR_LEN = 12; //fixed index for len
    const int NAME_INDEX = 53; //fixed index for name

    bool ok = true;
    QString sizeStr = str.mid(SIZE_INDEX, SIZE_STR_LEN);
    sizeStr.remove(QRegularExpression("\\s*"));
    qint64 size = sizeStr.toULongLong(&ok);
    if (!ok || str.length() <= NAME_INDEX)
    {
        currentTaskParams->task->SetError(QObject::tr("Unknown format of archiver output"));
        return;
    }

    QString file = str.right(str.size() - NAME_INDEX);
    Q_ASSERT(!currentTaskParams->filesAndSizes.contains(file));
    currentTaskParams->filesAndSizes[file] = size;
}

void ZipTaskProcessor::ProcessOutputForUnpack(const QByteArray& line)
{
    QString str(line);
    QString startStr("- ");
    if (!str.startsWith(startStr))
    {
        return;
    }
    str.remove(0, startStr.length());
    if (currentTaskParams->filesAndSizes.contains(str))
    {
        currentTaskParams->matchedSize += currentTaskParams->filesAndSizes[str];
    }
    float progress = (currentTaskParams->matchedSize * 100.0f) / currentTaskParams->totalSize;
    int progressInt = static_cast<int>(qRound(progress));
    currentTaskParams->notifier.NotifyProgress(currentTaskParams->task.get(), progressInt);
}

QString ZipTaskProcessor::GetArchiverPath() const
{
    static QString processAddr = qApp->applicationDirPath() +
#if defined(Q_OS_WIN)
    "/7z.exe";
#elif defined Q_OS_MAC
    "/../Resources/7za";
#endif //Q_OS_MAC Q_OS_WIN
    return processAddr;
}

void ZipTaskProcessor::CheckProgramState() const
{
    QString processAddr = GetArchiverPath();
    if (!QFile::exists(processAddr))
    {
        currentTaskParams->task->SetError(QObject::tr("Archiver tool was not found. Please, reinstall application"));
    }
    else
    {
        QString archivePath = currentTaskParams->task->GetArchivePath();
        if (archivePath.endsWith(".zip") == false)
        {
            currentTaskParams->task->SetError(QObject::tr("Required file is not a zip archive"));
        }
        else if (QFile::exists(archivePath) == false)
        {
            currentTaskParams->task->SetError(QObject::tr("Required archive doesn't exists"));
        }
    }
}
