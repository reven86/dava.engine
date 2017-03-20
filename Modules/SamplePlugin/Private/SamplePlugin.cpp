#include <PluginManager/Plugin.h>
#include <ModuleManager/IModule.h>
#include <Reflection/ReflectionRegistrator.h>
#include <UI/Components/UIComponent.h>
#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>

using namespace DAVA;

int DAVAMain(Vector<String> cmdline)
{
    return 0;
}

class SamplePluginUIComponent : public UIBaseComponent<SamplePluginUIComponent>
{
    DAVA_VIRTUAL_REFLECTION(SampleModuleUIComponent, UIComponent);

    UIComponent* Clone() const override
    {
        return new SamplePluginUIComponent(*this);
    }
};


DAVA_VIRTUAL_REFLECTION_IMPL(SamplePluginUIComponent)
{
    ReflectionRegistrator<SamplePluginUIComponent>::Begin()
        .ConstructorByPointer()
        .DestructorByPointer([] (SamplePluginUIComponent* o) { o->Release(); })
        .End();
}

class SamplePlugin : public IModule
{
public:
    enum eStatus
    {
        ES_UNKNOWN,
        ES_INIT,
        ES_SHUTDOWN
    };

    SamplePlugin(Engine* engine)
        : IModule(engine)
    {
        statusList.emplace_back(eStatus::ES_UNKNOWN);

        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SamplePluginUIComponent);
        engine->GetContext()->componentManager->RegisterComponent<SamplePluginUIComponent>();

        UIComponent* c = UIComponent::CreateByType(Type::Instance<SamplePluginUIComponent>());
        c->Release();
    }

    ~SamplePlugin()
    {
    }

    void Init() override
    {
        statusList.emplace_back(eStatus::ES_INIT);
    }

    void Shutdown() override
    {
        statusList.emplace_back(eStatus::ES_SHUTDOWN);
    }

private:
    Vector<eStatus> statusList;
};

EXPORT_PLUGIN(SamplePlugin)
