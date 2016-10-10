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
        .Method("sum2", &ReflClass::sum2)
        .Method("sum3", &ReflClass::sum3)
        .Method("sum4", &ReflClass::sum4)
        .Method("sum5", &ReflClass::sum5)
        .Method("sum6", &ReflClass::sum6)
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

    DAVA::int32 sum2(DAVA::int32 a1, DAVA::int32 a2)
    {
        return a1 + a2;
    }

    DAVA::int32 sum3(DAVA::int32 a1, DAVA::int32 a2, DAVA::int32 a3)
    {
        return a1 + a2 + a3;
    }

    DAVA::int32 sum4(DAVA::int32 a1, DAVA::int32 a2, DAVA::int32 a3, DAVA::int32 a4)
    {
        return a1 + a2 + a3 + a4;
    }

    DAVA::int32 sum5(DAVA::int32 a1, DAVA::int32 a2, DAVA::int32 a3, DAVA::int32 a4, DAVA::int32 a5)
    {
        return a1 + a2 + a3 + a4 + a5;
    }

    DAVA::int32 sum6(DAVA::int32 a1, DAVA::int32 a2, DAVA::int32 a3, DAVA::int32 a4, DAVA::int32 a5, DAVA::int32 a6)
    {
        return a1 + a2 + a3 + a4 + a5 + a6;
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
    END_FILES_COVERED_BY_TESTS();

    DAVA_TEST (FullTest)
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
    -- DV functions
    DV.Debug("Debug msg")
    DV.Error("Error msg")

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
    --assert(subClass.colorVal == context.colorVal, "Test fail! subClass.colorVal (" ..  tostring(subClass.colorVal) .. ") != context.colorVal (" .. tostring(context.colorVal) .. ")")

    -- Test methods
    local invertedValue = context.invert(42)
    assert(invertedValue == -42, "Test fail! context.invert(42) '" .. invertedValue .. "' != -42")
    local sum = context.sum2(1, 2)
    assert(invertedValue == 3, "Test fail! context.sum2 '" .. sum .. "' != 3")
    sum = context.sum3(1, 2, 3)
    assert(invertedValue == 6, "Test fail! context.sum3 '" .. sum .. "' != 6")
    sum = context.sum4(1, 2, 3, 4)
    assert(invertedValue == 10, "Test fail! context.sum4 '" .. sum .. "' != 10")
    sum = context.sum5(1, 2, 3, 4, 5)
    assert(invertedValue == 15, "Test fail! context.sum5 '" .. sum .. "' != 15")
    sum = context.sum6(1, 2, 3, 4, 5, 6)
    assert(invertedValue == 21, "Test fail! context.sum6 '" .. sum .. "' != 21")
    
    -- tostring tests
    assert(tostring(context.colorVal) != "", "Test fail! tostring(Any) is empty!")
    assert(tostring(context.invert) != "", "Test fail! tostring(AnyFn) is empty!")
    assert(tostring(context) != """, "Test fail! tostring(Reflection) is empty!")
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
    }

    DAVA_TEST (CompileStringErrorTest)
    {
        DAVA::LuaScript s;

        const DAVA::String error_script = R"script(
incorrect script
)script";

        // Check exception
        try
        {
            s.ExecString(error_script);
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() != 0);
        }

        // Check safe method
        TEST_VERIFY(s.ExecStringSafe(error_script) < 0);
    }

    DAVA_TEST (ExecStringErrorTest)
    {
        DAVA::LuaScript s;

        const DAVA::String error_script = R"script(
undefined_function_call("test")
)script";

        // Check exception
        try
        {
            s.ExecString(error_script);
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() != 0);
        }

        // Check safe method
        TEST_VERIFY(s.ExecStringSafe(error_script) < 0);
    }

    DAVA_TEST (ExecFunctionErrorTest)
    {
        DAVA::LuaScript s;

        const DAVA::String error_script = R"script(
function main()
    undefined_function_call("test")
end
)script";

        TEST_VERIFY(s.ExecStringSafe(error_script) >= 0);

        // Check exception
        try
        {
            s.ExecFunction("main");
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() != 0);
        }

        // Check safe method
        TEST_VERIFY(s.ExecFunctionSafe("main") < 0);
    }

    DAVA_TEST (MoveConstructorTest)
    {
        DAVA::LuaScript s;

        const DAVA::String easy_script = R"script(
function main()
end
)script";

        TEST_VERIFY(s.ExecStringSafe(easy_script) >= 0);

        DAVA::LuaScript moveScript(std::move(s));

        TEST_VERIFY(moveScript.ExecFunctionSafe("main") >= 0);
    }

    DAVA_TEST (PopResultsTest)
    {
        DAVA::LuaScript s;

        const DAVA::String easy_script = R"script(
return 42, "demo"
)script";

        DAVA::int32 nresults = s.ExecStringSafe(easy_script);
        TEST_VERIFY(nresults == 2);

        try
        {
            DAVA::Any r = s.PopResult();
            TEST_VERIFY(r.CanGet<DAVA::String>());
            TEST_VERIFY(r.Get<DAVA::String>() == "demo");
        }
        catch (const DAVA::LuaException&)
        {
            TEST_VERIFY(false);
        }

        DAVA::Any r;
        TEST_VERIFY(s.PopResultSafe(r));
        TEST_VERIFY(!r.IsEmpty());
        TEST_VERIFY(r.CanGet<DAVA::float64>());
        TEST_VERIFY(FLOAT_EQUAL(r.Get<DAVA::float64>(), 42.f));
    }

    DAVA_TEST (PopResultsErrorTest)
    {
        DAVA::LuaScript s;

        const DAVA::String easy_script = R"script(
--do nothing
)script";

        DAVA::int32 nresults = s.ExecStringSafe(easy_script);
        TEST_VERIFY(nresults == 0);

        try
        {
            DAVA::Any r = s.PopResult();
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() != 0);
        }

        DAVA::Any r;
        TEST_VERIFY(!s.PopResultSafe(r));
        TEST_VERIFY(r.IsEmpty());
    }

    DAVA_TEST (LuaExceptionTest)
    {
        try
        {
            DAVA_THROW(DAVA::LuaException, -1, DAVA::String("Some error"));
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() == -1);
        }

        try
        {
            DAVA_THROW(DAVA::LuaException, -1, static_cast<const char*>("Some error"));
            TEST_VERIFY(false);
        }
        catch (const DAVA::LuaException& e)
        {
            TEST_VERIFY(e.ErrorCode() == -1);
        }
    }
};