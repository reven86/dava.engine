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

struct SamplePluginA : public ReflectionBase
{
    SamplePluginA(const String& name_ = String())
        : name(name_)
    {
        if (name.empty())
        {
            name = String("SamplePluginA");
        }
    }

    int ai = 11111;
    float af = 54321;
    String name;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SamplePluginA)
    {
        ReflectionRegistrator<SamplePluginA>::Begin()
        .Field("ai", &SamplePluginA::ai)
        .Field("af", &SamplePluginA::af)
        .Field("name", &SamplePluginA::name)
        .End();
    }
};

struct SamplePluginB : public SamplePluginA
{
    SamplePluginB()
        : SamplePluginA("SamplePluginB")
    {
    }

    int bi = 22222;
    float bf = 98765;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SamplePluginB, SamplePluginA)
    {
        ReflectionRegistrator<SamplePluginB>::Begin()
        .Field("bi", &SamplePluginB::bi)
        .Field("bf", &SamplePluginB::bf)
        .End();
    }
};

class SamplePluginUIComponent : public UIBaseComponent<SamplePluginUIComponent>
{
    DAVA_VIRTUAL_REFLECTION(SamplePluginUIComponent, UIComponent);

public:
    SamplePluginUIComponent()
        : a_ptr(new SamplePluginB())
    {
    }

    ~SamplePluginUIComponent()
    {
        delete a_ptr;
    }

    SamplePluginA a;
    SamplePluginB* a_ptr;

    UIComponent* Clone() const override
    {
        return new SamplePluginUIComponent(*this);
    }
};

DAVA_VIRTUAL_REFLECTION_IMPL(SamplePluginUIComponent)
{
    ReflectionRegistrator<SamplePluginUIComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](SamplePluginUIComponent* o) { o->Release(); })
    .Field("a", &SamplePluginUIComponent::a)
    .Field("a_ptr", &SamplePluginUIComponent::a_ptr)
    .Method("Clone", &SamplePluginUIComponent::Clone)
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
        TypeDB::GetLocalDB()->SetMasterDB(engine->GetContext()->typeDB);
        FastNameDB::GetLocalDB()->SetMasterDB(engine->GetContext()->fastNameDB);
        ReflectedTypeDB::GetLocalDB()->SetMasterDB(engine->GetContext()->reflectedTypeDB);

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
