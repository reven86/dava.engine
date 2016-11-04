#include "Classes/Qt/Application/InitModule.h"

#include "Classes/Qt/Scene/Selectable.h"
#include "Classes/Qt/Scene/BaseTransformProxies.h"

#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitterInstance.h"

InitModule::~InitModule()
{
    Selectable::RemoveAllTransformProxies();
}

void InitModule::OnContextCreated(DAVA::TArc::DataContext& context)
{
}

void InitModule::OnContextDeleted(DAVA::TArc::DataContext& context)
{
}

void InitModule::PostInit()
{
    Selectable::AddTransformProxyForClass<DAVA::Entity, EntityTransformProxy>();
    Selectable::AddTransformProxyForClass<DAVA::ParticleEmitterInstance, EmitterTransformProxy>();
}
