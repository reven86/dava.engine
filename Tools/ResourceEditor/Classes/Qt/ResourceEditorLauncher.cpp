#include "ResourceEditorLauncher.h"

ResourceEditorLauncher::~ResourceEditorLauncher()
{
    Selectable::RemoveAllTransformProxies();
}

void ResourceEditorLauncher::Launch()
{
    Selectable::AddTransformProxyForClass<DAVA::Entity, EntityTransformProxy>();
    Selectable::AddTransformProxyForClass<DAVA::ParticleEmitterInstance, EmitterTransformProxy>();
    DelayedExecute(DAVA::MakeFunction(this, &ResourceEditorLauncher::LaunchImpl));
}

void ResourceEditorLauncher::LaunchImpl()
{
    DVASSERT(ProjectManager::Instance() != nullptr);
    connect(ProjectManager::Instance(), &ProjectManager::ProjectOpened, this, &ResourceEditorLauncher::OnProjectOpened, Qt::QueuedConnection);
    ProjectManager::Instance()->OpenLastProject();
}

void ResourceEditorLauncher::OnProjectOpened(const QString&)
{
    DVASSERT(ProjectManager::Instance() != nullptr);
    disconnect(ProjectManager::Instance(), &ProjectManager::ProjectOpened, this, &ResourceEditorLauncher::OnProjectOpened);

    DAVA::uint32 val = SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32();
    DAVA::eGPUFamily family = static_cast<DAVA::eGPUFamily>(val);
    DAVA::Texture::SetGPULoadingOrder({ family });

    emit LaunchFinished();
}
