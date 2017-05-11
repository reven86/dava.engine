#pragma once

#include "Core/Tasks/AsyncChainTask.h"

#include <QVector>
#include <QUrl>
#include <QString>

class UpdateConfigTask : public AsyncChainTask
{
public:
    UpdateConfigTask(ApplicationManager* appManager, const QVector<QUrl>& urls);

private:
    QString GetDescription() const override;
    void Run() override;

    void OnConfigLoaded(const BaseTask* task);
    QVector<QUrl> urls;
};
