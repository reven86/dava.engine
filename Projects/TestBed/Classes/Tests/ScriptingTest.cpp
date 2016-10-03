#include "Tests/ScriptingTest.h"
#include "Base/Type.h"
#include "Reflection/Registrator.h"
#include "Scripting/LuaScript.h"
#include "UI/Input/UIActionBindingComponent.h"
#include "UI/Input/UIActionMap.h"
#include "Utils/StringUtils.h"

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

DemoObj demoObj;
Reflection objRef;
LuaScript* script = nullptr;

static const String demo_script = R"script(DV.Debug("LUA: Script loaded")
function main(int, str, ref)
    DV.Debug("LUA: Main function")
    return int, str, ref
end
return true -- Return in global body works fine
)script";

ScriptingTest::ScriptingTest(GameCore* g)
    : BaseScreen(g, "ScriptingTest")
{
}

void ScriptingTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/ScriptingTest.yaml", &pkgBuilder);
    UIControl* dialog = pkgBuilder.GetPackage()->GetControl(0);
    AddControl(dialog);

    scriptText = static_cast<UITextField*>(dialog->FindByName("ScriptText"));
    intArgText = static_cast<UITextField*>(dialog->FindByName("IntArgText"));
    strArgText = static_cast<UITextField*>(dialog->FindByName("StrArgText"));
    outputText = static_cast<UIStaticText*>(dialog->FindByName("OutputText"));
    timeText = static_cast<UIStaticText*>(dialog->FindByName("TimeText"));

    scriptText->SetUtf8Text(demo_script);
    intArgText->SetUtf8Text("42");
    strArgText->SetUtf8Text("demoStr");
    outputText->SetUtf8Text("");

    UIActionMap& amap = dialog->GetOrCreateComponent<UIActionBindingComponent>()->GetActionMap();
    amap.Put(FastName("LOAD_SCRIPT"), [&]() {
        try
        {
            uint64 begin = SystemTimer::Instance()->GetAbsoluteUs();
            int32 nresults = script->ExecuteString(scriptText->GetUtf8Text());
            uint64 time = SystemTimer::Instance()->GetAbsoluteUs() - begin;

            String output = Format("Execute script time: %llu us\n", time);
            for (int32 i = 0; i < nresults; ++i)
            {
                Any val = script->PopAny();
                output += Format("%d) %s\n", i, val.IsEmpty() ? "<empty>" : val.GetType()->GetName());
            }
            outputText->SetUtf8Text(output);
            timeText->SetUtf8Text(Format("Time: %llu us", time));
        }
        catch (const LuaException& e)
        {
            String error = Format("LuaException: %s", e.what());
            Logger::Error(error.c_str());
            outputText->SetUtf8Text(error);
            timeText->SetUtf8Text("Error");
        }
    });
    amap.Put(FastName("RUN_MAIN"), [&]() {
        int32 intArg = atoi(intArgText->GetUtf8Text().c_str());
        String strArg = strArgText->GetUtf8Text();
        try
        {
            uint64 begin = SystemTimer::Instance()->GetAbsoluteUs();
            int32 nresults = script->CallFunction("main", intArg, strArg, objRef);
            uint64 time = SystemTimer::Instance()->GetAbsoluteUs() - begin;

            String output = Format("Run main(...) time: %llu us\n", time);
            for (int32 i = 0; i < nresults; ++i)
            {
                Any val = script->PopAny();
                output += Format("%d) %s\n", i, val.IsEmpty() ? "<empty>" : val.GetType()->GetName());
            }
            outputText->SetUtf8Text(output);
            timeText->SetUtf8Text(Format("Time: %llu us", time));
        }
        catch (const LuaException& e)
        {
            String error = Format("LuaException: %s", e.what());
            Logger::Error(error.c_str());
            outputText->SetUtf8Text(error);
            timeText->SetUtf8Text("Error");
        }
    });
    amap.Put(FastName("RUN_MAIN_NOARGS"), [&]() {
        try
        {
            uint64 begin = SystemTimer::Instance()->GetAbsoluteUs();
            int32 nresults = script->CallFunction("main");
            uint64 time = SystemTimer::Instance()->GetAbsoluteUs() - begin;

            String output = Format("Run main() time: %llu us\n", time);
            for (int32 i = 0; i < nresults; ++i)
            {
                Any val = script->PopAny();
                output += Format("%d) %s\n", i, val.IsEmpty() ? "<empty>" : val.GetType()->GetName());
            }
            outputText->SetUtf8Text(output);
            timeText->SetUtf8Text(Format("Time: %llu us", time));
        }
        catch (const LuaException& e)
        {
            String error = Format("LuaException: %s", e.what());
            Logger::Error(error.c_str());
            outputText->SetUtf8Text(error);
            timeText->SetUtf8Text("Error");
        }
    });
    amap.Put(FastName("RESET_SCRIPT"), [&]() {
        SafeDelete(script);
        script = new LuaScript();
        script->SetGlobalVariable("GlobRef", objRef);
    });
    amap.Put(FastName("RUN_10000"), [&]() {
        int32 intArg = atoi(intArgText->GetUtf8Text().c_str());
        String strArg = strArgText->GetUtf8Text();
        try
        {
            uint64 begin = SystemTimer::Instance()->GetAbsoluteUs();
            for (int32 i = 0; i < 10000; ++i)
            {
                script->CallFunction("main", intArg, strArg, objRef);
            }
            uint64 time = SystemTimer::Instance()->GetAbsoluteUs() - begin;

            outputText->SetUtf8Text(Format("Run 10k main() time: %llu us", time));
            timeText->SetUtf8Text(Format("Time: %llu us", time));
        }
        catch (const LuaException& e)
        {
            String error = Format("LuaException: %s", e.what());
            Logger::Error(error.c_str());
            outputText->SetUtf8Text(error);
            timeText->SetUtf8Text("Error");
        }
    });

    demoObj.v.assign({ 1, 2, 3, 4, 5 });
    objRef = Reflection::Create(&demoObj).ref;
    script = new LuaScript();
    script->SetGlobalVariable("GlobRef", objRef);
}

void ScriptingTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    SafeDelete(script);
}

void ScriptingTest::Update(DAVA::float32 timeElapsed)
{
}
