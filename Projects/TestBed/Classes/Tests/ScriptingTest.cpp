#include "Tests/ScriptingTest.h"
#include "Base/Type.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scripting/Script.h"

class DemoObj
{
    DAVA_DECLARE_TYPE_INITIALIZER;

public:
    int a = 99;
    String b = "DEMO_TEST";
    Color c = Color::White;
};

DAVA_TYPE_INITIALIZER(DemoObj)
{
    DAVA::ReflectionRegistrator<DemoObj>::Begin()
    .Field("a", &DemoObj::a)
    .Field("b", &DemoObj::b)
    .Field("c", &DemoObj::c)
    .End();
}

using namespace DAVA;

ScriptingTest::ScriptingTest()
    : BaseScreen("ScriptingTest")
{
}

void ScriptingTest::LoadResources()
{
    BaseScreen::LoadResources();

    static const String demo_script =
    R"script(

DV.Debug("LUA: static code")

function printRef(any)
    DV.Debug("Print any: " .. tostring(any:value()))
end

function main(context)
    aRef = context:ref("a")
    printRef(aRef)
    bRef = aRef
    aRef:set(956)
    printRef(bRef)

    cRef = context:ref("c")
    DV.Debug("C ref: " .. tostring(cRef))
    cVal = cRef:value()
    DV.Debug("C val: " .. tostring(cVal))

    DV.Debug("type(aRef): " .. type(aRef))
    DV.Debug("type(cRef): " .. type(cRef))
    DV.Debug("type(cVal): " .. type(cVal))
end

)script";

    DemoObj obj;
    DAVA::Reflection objRef = DAVA::Reflection::Reflect(&obj);

    Script s;
    s.LoadString(demo_script);
    s.Run(objRef);
}

void ScriptingTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}
