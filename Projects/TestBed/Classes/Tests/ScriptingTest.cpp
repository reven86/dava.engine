#include "Tests/ScriptingTest.h"
#include "Base/Type.h"
#include "Reflection/Registrator.h"
#include "Scripting/Script.h"

using namespace DAVA;

class SubObj : public ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(SubObj)
    {
        ReflectionRegistrator<SubObj>::Begin()
        .Field("a", &SubObj::a)
        .Field("b", &SubObj::b)
        .Field("c", &SubObj::c)
        .End();
    }

public:
    int32 a = 10;
    WideString b = L"WideString";
    Color c = Color::Black;
};

class DemoObj : public ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(DemoObj)
    {
        ReflectionRegistrator<DemoObj>::Begin()
        .Field("a", &DemoObj::a)
        .Field("b", &DemoObj::b)
        .Field("c", &DemoObj::c)
        .Field("d", &DemoObj::d)
        .End();
    }

public:
    int32 a = 99;
    String b = "String";
    Color c = Color::White;
    SubObj d;
};

ScriptingTest::ScriptingTest(GameCore* g)
    : BaseScreen(g, "ScriptingTest")
{
}

void ScriptingTest::LoadResources()
{
    BaseScreen::LoadResources();

    static const String demo_script = R"script(

function main(context)
    DV.Debug("context: "..tostring(context))
    DV.Debug("context.a: "..tostring(context.a))
    DV.Debug("context.b: "..tostring(context.b))
    DV.Debug("context.c: "..tostring(context.c))
    DV.Debug("context.d: "..tostring(context.d))
    DV.Debug("context.d.a: "..tostring(context.d.a))
    DV.Debug("context.d.b: "..tostring(context.d.b))
    DV.Debug("context.d.c: "..tostring(context.d.c))

    context.a = 1
    DV.Debug("context.a: "..tostring(context.a))

    context.b = "New String"
    DV.Debug("context.b: "..tostring(context.b))

    context.c = context.d.c
    DV.Debug("context.c: "..tostring(context.c))

    context.d.a = 2    
    DV.Debug("context.d.a: "..tostring(context.d.a))

    context.d.b = "New WideString"
    DV.Debug("context.d.b: "..tostring(context.d.b))


    DV.Debug("context.a + context.d.a: "..(context.a + context.d.a))

end

)script";

    DemoObj obj;
    Reflection objRef = Reflection::Create(&obj).ref;

    Script s;
    s.LoadString(demo_script);
    s.Run(objRef);
}

void ScriptingTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}
