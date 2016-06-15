#ifndef __RESOURCEEDITOR_LAUNCHER_H__
#define __RESOURCEEDITOR_LAUNCHER_H__

#include "Scene/BaseTransformProxies.h"
#include "Project/ProjectManager.h"
#include "Main/mainwindow.h"

#include <QObject>

class ResourceEditorLauncher : public QObject
{
    Q_OBJECT

public slots:
    void Launch();
    void OnProjectOpened(const QString&);

public:
    ~ResourceEditorLauncher();
};

inline void ResourceEditorLauncher::Launch()
{
    Selectable::AddTransformProxyForClass<DAVA::Entity, EntityTransformProxy>();
    Selectable::AddTransformProxyForClass<DAVA::ParticleEmitterInstance, EmitterTransformProxy>();

    DVASSERT(ProjectManager::Instance() != nullptr);
    connect(ProjectManager::Instance(), &ProjectManager::ProjectOpened, this, &ResourceEditorLauncher::OnProjectOpened, Qt::QueuedConnection);
    ProjectManager::Instance()->OpenLastProject();
}

inline ResourceEditorLauncher::~ResourceEditorLauncher()
{
    Selectable::RemoveAllTransformProxies();
}

inline void ResourceEditorLauncher::OnProjectOpened(const QString&)
{
    DVASSERT(ProjectManager::Instance() != nullptr);
    disconnect(ProjectManager::Instance(), &ProjectManager::ProjectOpened, this, &ResourceEditorLauncher::OnProjectOpened);

    DVASSERT(QtMainWindow::Instance() != nullptr);
    QtMainWindow::Instance()->SetupTitle();
    QtMainWindow::Instance()->OnSceneNew();
}

#endif // __RESOURCEEDITOR_LAUNCHER_H__
