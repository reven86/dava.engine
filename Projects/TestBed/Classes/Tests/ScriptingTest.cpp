#include "Tests/ScriptingTest.h"
#include "Base/Type.h"
#include "Reflection/Registrator.h"
#include "Scripting/LuaScript.h"

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
        .Field("v", &DemoObj::v)
        .End();
    }

public:
    int32 a = 99;
    String b = "String";
    Color c = Color::White;
    SubObj d;
    Vector<int32> v;
};

static const String demo_script = R"script(

function main(context)
    DV.Debug("GlobObj.a: "..tostring(GlobObj.a))
    
    DV.Debug("context: "..tostring(context))
    DV.Debug("context.a: "..tostring(context.a))
    DV.Debug("context[\"a\"]: "..tostring(context["a"]))
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

    DV.Debug("context.v: "..tostring(context.v))
    DV.Debug("Length #context.v: "..tostring(#context.v))
    context.v[3] = 999;
    DV.Debug("----- index for -----")
    for i = 1, #context.v do
        DV.Debug("  context.v["..i.."]: "..context.v[i])
    end
end

)script";

static const String sss = R"script(

glob_value = 1

function main(arg1, arg2, arg3, arg4)
    DV.Debug(tostring(arg1))
    DV.Debug(tostring(arg2))
    DV.Debug(tostring(arg3))
    DV.Debug(tostring(arg4))
    return glob_value
end

)script";

ScriptingTest::ScriptingTest(GameCore* g)
    : BaseScreen(g, "ScriptingTest")
{
}

void ScriptingTest::LoadResources()
{
    BaseScreen::LoadResources();

    DemoObj obj;
    obj.v.assign({ 1, 2, 3, 4, 5 });

    Reflection objRef = Reflection::Create(&obj).ref;

    LuaScript s;
    s.RunString(sss);
    Any args[] = { 1, "String", false, 16.5f };
    LuaScript::RunResult res = s.RunMain(args);

    float64 d = res.value.Cast<float64>();
    float32 f = res.value.Cast<float32>();
    int32 i = res.value.Cast<int32>();

    DAVA::Logger::Debug("CPP: obj.v[3] == %d", obj.v[3]);
}

void ScriptingTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}

void ScriptingTest::Update(DAVA::float32 timeElapsed)
{
}
