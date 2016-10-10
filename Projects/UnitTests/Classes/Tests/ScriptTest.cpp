#include "UnitTests/UnitTests.h"
#include "Base/BaseTypes.h"
#include "Math/Color.h"
#include "Math/MathDefines.h"
#include "Reflection/Registrator.h"
#include "Scripting/LuaScript.h"

struct ReflClass : public DAVA::ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(ReflClass)
    {
        DAVA::ReflectionRegistrator<ReflClass>::Begin()
        .Field("intVal", &ReflClass::intVal)
        .Field("floatVal", &ReflClass::floatVal)
        .Field("stringVal", &ReflClass::stringVal)
        .Field("boolVal", &ReflClass::boolVal)
        .Field("colorVal", &ReflClass::colorVal)
        .Field("subClass", &ReflClass::subClass)
        .Method("invert", &ReflClass::invert)
        .End();
    }

    ReflClass()
        : colorVal(DAVA::Color::White)
    {
    }

    DAVA::int32 invert(DAVA::int32 value)
    {
        return -value;
    }

public:
    DAVA::int32 intVal = 0;
    DAVA::float32 floatVal = 0.0f;
    bool boolVal = false;
    DAVA::String stringVal;
    DAVA::Color colorVal;
    ReflClass* subClass = nullptr;
};

DAVA_TESTCLASS (ScriptTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(DavaFramework)
    DECLARE_COVERED_FILES("LuaScript.cpp")
    DECLARE_COVERED_FILES("LuaException.cpp")
    DECLARE_COVERED_FILES("LuaBridge.cpp")
    END_FILES_COVERED_BY_TESTS()

    DAVA_TEST (BasicTest)
    {
        ReflClass subcl;
        subcl.intVal = 10;
        ReflClass cl;
        cl.intVal = 5;
        cl.floatVal = 1.4f;
        cl.boolVal = true;
        cl.stringVal = "Demo string";
        cl.colorVal = DAVA::Color::Black;
        cl.subClass = &subcl;
        DAVA::Reflection clRef = DAVA::Reflection::Create(&cl);

        DAVA::LuaScript s;

        /* Valid test */

        const DAVA::String script = R"script(
function main(context)
    -- Get value tests
    local intVal = context.intVal
    assert(intVal == 5, "Test fail! context.intVal " .. intVal .. " != 5")
    local floatVal = context.floatVal
    assert(math.abs(floatVal - 1.4) <= 0.00001, "Test fail! context.floatVal " .. floatVal .. " != 1.4")
    local boolVal = context.boolVal
    assert(boolVal == true, "Test fail! context.boolVal " .. tostring(boolVal) .. " != true")
    local stringVal = context.stringVal
    assert(stringVal == "Demo string", "Test fail! context.stringVal '" .. stringVal .. "' != 'Demo string'")
        
    -- Get global value
    intVal = GlobRef.intVal
    assert(intVal == 5, "Test fail! context.intVal " .. intVal .. " != 5")

    local subClass = context.subClass
    intVal = subClass.intVal
    assert(intVal == 10, "Test fail! subClass.intVal " .. intVal .. " != 10")

    -- Set value tests
    context.intVal = 42
    assert(context.intVal == 42, "Test fail! context.intVal " .. context.intVal .. " != 42")

    context.floatVal = 3.14
    assert(math.abs(context.floatVal - 3.14) <= 0.00001, "Test fail! context.floatVal " .. context.floatVal .. " != 3.14")

    context.boolVal = false
    assert(context.boolVal == false, "Test fail! context.boolVal " .. tostring(context.boolVal) .. " != false")

    context.stringVal = "New demo string"
    assert(context.stringVal == "New demo string", "Test fail! context.stringVal '" .. context.stringVal .. "' != 'New demo string'")
    
    -- Test complex type DAVA::Color as userdata
    subClass.colorVal = context.colorVal
    --assert(subClass.colorVal == context.colorVal, "Test fail!  subClass.colorVal (" ..  tostring(subClass.colorVal) .. ") != context.colorVal (" .. tostring(context.colorVal) .. ")")

    -- Test methods
    local invertedValue = context.invert(42);
    assert(invertedValue == -42, "Test fail! invertedValue '" .. invertedValue .. "' != -42")
end
)script";

        s.SetGlobalVariable("GlobRef", clRef);
        TEST_VERIFY(s.ExecStringSafe(script) >= 0);
        TEST_VERIFY(s.ExecFunctionSafe("main", clRef) >= 0);
        TEST_VERIFY(cl.intVal == 42);
        TEST_VERIFY(FLOAT_EQUAL(cl.floatVal, 3.14f));
        TEST_VERIFY(cl.boolVal == false);
        TEST_VERIFY(cl.stringVal == "New demo string");
        TEST_VERIFY(cl.colorVal == subcl.colorVal);

        /* Run from string error */

        const DAVA::String error_script = R"script(
incorrect script
)script";

        try
        {
            s.ExecString(error_script);
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() != 0);
        }

        /* Run main function error */

        const DAVA::String error_script2 = R"script(
function main()
    undefined_function_call("test")
end
)script";

        TEST_VERIFY(s.ExecStringSafe(error_script2) >= 0);
        try
        {
            s.ExecFunction("main");
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() != 0);
        }
    }
};
