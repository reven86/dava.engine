#pragma once

#include "Scene/BaseTransformProxies.h"
#include "Project/ProjectManager.h"
#include "Main/mainwindow.h"

#include "QtTools/Utils/QtDelayedExecutor.h"

class ResourceEditorLauncher : public QtDelayedExecutor
{
    Q_OBJECT

public:
    ~ResourceEditorLauncher();

    void Launch();

private:
    void LaunchImpl();
    void OnProjectOpened(const QString&);

public:
    Q_SIGNAL void LaunchFinished();
};
