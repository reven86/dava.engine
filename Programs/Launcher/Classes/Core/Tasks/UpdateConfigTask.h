#pragma once

#include "Core/Tasks/AsyncChainTask.h"

#include <QUrl>
#include <QString>

class UpdateConfigTask : public AsyncChainTask
{
public:
    UpdateConfigTask(ApplicationManager* appManager, const std::vector<QUrl>& urls);

private:
    QString GetDescription() const override;
    void Run() override;

    void OnConfigLoaded(const BaseTask* task);
    std::vector<QUrl> urls;
};
