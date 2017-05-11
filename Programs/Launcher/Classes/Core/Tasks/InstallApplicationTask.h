#pragma once

#include "Core/Tasks/AsyncChainTask.h"
#include "Data/ConfigParser.h"

struct InstallApplicationParams
{
    QString branch;
    QString app;
    AppVersion* currentVersion;
    AppVersion newVersion;
};

class InstallApplicationTask final : public AsyncChainTask
{
public:
    InstallApplicationTask(ApplicationManager* appManager, const InstallApplicationParams& params);

private:
    QString GetDescription() const override;
    void Run() override;

    void OnLoaded(const BaseTask* task);
    void Install();
    void OnInstalled();

    QStringList GetApplicationsToRestart(const QString& branchID, const QString& appID, const AppVersion* installedVersion);
    bool CanTryStopApplication(const QString& applicationName) const;

    QString GetAppName() const;

    InstallApplicationParams params;
    QStringList applicationsToRestart;
};
