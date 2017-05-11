#pragma once

#include "Core/Tasks/BaseTask.h"

class RunApplicationTask : public RunTask
{
public:
    RunApplicationTask(ApplicationManager* appManager, const QString& branch, const QString& app, const QString& version);

private:
    QString GetDescription() const override;
    void Run() override;

    QString branch;
    QString app;
    QString version;
};
