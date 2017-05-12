#include "Core/Tasks/UnzipTask.h"
#include <QObject>

UnzipTask::UnzipTask(ApplicationManager* appManager, const QString& archivePath, const QString& outputPath)
    : BaseTask(appManager)
    , archivePath(archivePath)
    , outputPath(outputPath)
{
}

QString UnzipTask::GetDescription() const
{
    return QObject::tr("Unpacking archive");
}

BaseTask::eTaskType UnzipTask::GetTaskType() const
{
    return ZIP_TASK;
}

QString UnzipTask::GetArchivePath() const
{
    return archivePath;
}

QString UnzipTask::GetOutputPath() const
{
    return outputPath;
}
