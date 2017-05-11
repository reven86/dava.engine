#pragma once

#include "Core/Tasks/BaseTask.h"
#include "Data/ConfigParser.h"

class RemoveApplicationTask : public RunTask
{
public:
    RemoveApplicationTask(ApplicationManager* appManager, const QString& branch, const QString& app);

private:
    QString GetDescription() const override;
    void Run() override;

    bool TryStopApp(const QString& runPath) const;
    bool CanRemoveApp(const QString& branchID, const QString& appID, AppVersion* localVersion) const;
    bool RemoveApplicationImpl(const QString& branchID, const QString& appID, AppVersion* localVersion);

    QString branch;
    QString app;
};
