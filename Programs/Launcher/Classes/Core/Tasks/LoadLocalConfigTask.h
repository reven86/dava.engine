#pragma once

#include "Core/Tasks/BaseTask.h"

class LoadLocalConfigTask : public RunTask
{
public:
    LoadLocalConfigTask(ApplicationManager* appManager, const QString& localConfigPath);

private:
    QString GetDescription() const override;

    void Run() override;

    QString localConfigPath;
};
