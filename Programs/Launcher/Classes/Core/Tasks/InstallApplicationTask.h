#pragma once

#include "Core/Tasks/AsyncChainTask.h"
#include "Data/ConfigParser.h"

struct InstallApplicationParams
{
    QString branch;
    QString app;
    AppVersion newVersion;
};

class InstallApplicationTask final : public AsyncChainTask
{
public:
    InstallApplicationTask(ApplicationManager* appManager, const InstallApplicationParams& params);

private:
    enum eState
    {
        LOADING,
        UNPACKING,
        POST_INSTALL
    };

    QString GetDescription() const override;
    void Run() override;

    int GetSubtasksCount() const override;
    void OnFinished(const BaseTask* task) override;

    void OnLoaded(const BaseTask* task);
    void Install();
    void OnInstalled();

    QStringList GetApplicationsToRestart(const QString& branchID, const QString& appID);
    bool CanTryStopApplication(const QString& applicationName) const;

    QString GetAppName() const;

    bool NeedUnpack() const;

    InstallApplicationParams params;
    QStringList applicationsToRestart;

    eState state = LOADING;
};
