#pragma once

#include "Core/Tasks/AsyncChainTask.h"

class ApplicationQuitController;

class SelfUpdateTask : public AsyncChainTask
{
public:
    SelfUpdateTask(ApplicationManager* appManager, ApplicationQuitController* quitController, const QString& url);

private:
    QString GetDescription() const override;
    void Run() override;

    void OnLoaded(const BaseTask* task);
    void OnUnpacked();

    ApplicationQuitController* quitController = nullptr;
    QString url;
};
