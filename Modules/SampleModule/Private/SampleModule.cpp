#include "SampleModule.h"

#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SampleModuleUIComponent)
{
    ReflectionRegistrator<SampleModuleUIComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](SampleModuleUIComponent* o) { o->Release(); })
    .End();
}

SampleModule::SampleModule(Engine* engine)
    : IModule(engine)
{
    statusList.emplace_back(eStatus::ES_UNKNOWN);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SampleModuleUIComponent);
    GetEngineContext()->componentManager->RegisterComponent<SampleModuleUIComponent>();

    UIComponent* c = UIComponent::CreateByType(Type::Instance<SampleModuleUIComponent>());
    c->Release();
}

void SampleModule::Init()
{
    statusList.emplace_back(eStatus::ES_INIT);
}

void SampleModule::Shutdown()
{
    statusList.emplace_back(eStatus::ES_SHUTDOWN);
}
}