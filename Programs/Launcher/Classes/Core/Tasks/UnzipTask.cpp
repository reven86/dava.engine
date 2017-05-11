#include "Core/Tasks/UnzipTask.h"
#include <QObject>

UnzipTask::UnzipTask(ApplicationManager* appManager, const QString& archivePath, const QString& outputPath)
    : BaseTask(appManager)
    , archivePath(archivePath)
    , outputPath(outputPath)
{
    taskType = ZIP_TASK;
}

QString UnzipTask::GetDescription() const
{
    return QObject::tr("Unpacking archive");
}

QString UnzipTask::GetArchivePath() const
{
    return archivePath;
}

QString UnzipTask::GetOutputPath() const
{
    return outputPath;
}
