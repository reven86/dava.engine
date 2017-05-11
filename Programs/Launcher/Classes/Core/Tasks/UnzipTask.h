#pragma once

#include "Core/Tasks/BaseTask.h"

class UnzipTask final : public BaseTask
{
public:
    UnzipTask(ApplicationManager* appManager, const QString& archivePath, const QString& outputPath);

    QString GetDescription() const override;

    QString GetArchivePath() const;
    QString GetOutputPath() const;

private:
    QString archivePath;
    QString outputPath;
};
