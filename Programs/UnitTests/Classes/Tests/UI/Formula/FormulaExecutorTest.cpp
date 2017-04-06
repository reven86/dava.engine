#include "DAVAEngine.h"

#include "UI/Formula/FormulaParser.h"
#include "UI/Formula/FormulaExecutor.h"
#include "UI/Formula/FormulaError.h"
#include "UI/Formula/FormulaFormatter.h"

#include "Reflection/ReflectionRegistrator.h"

#include "UnitTests/UnitTests.h"

using namespace DAVA;

class TestData : public ReflectionBase
{
    DAVA_VIRTUAL_REFLECTION(TestData);

public:
    float flVal = 0.0f;
    bool bVal = false;
    String strVal = "Hello, world";
    int intVal = 42;
    Vector<int> array;
    Map<String, int> map;

    TestData()
    {
        array.push_back(10);
        array.push_back(20);
        array.push_back(30);

        map["a"] = 11;
        map["b"] = 22;
        map["c"] = 33;
    }

    int sum(int a, int b)
    {
        return a + b;
    }

    String intToStr(int a)
    {
        return Format("*%d*", a);
    }

    String boolToStr(bool a)
    {
        return a ? "+" : "-";
    }

    String floatToStr(float a)
    {
        return Format("%.3f", a);
    }
};

DAVA_VIRTUAL_REFLECTION_IMPL(TestData)
{
    ReflectionRegistrator<TestData>::Begin()
    .Field("fl", &TestData::flVal)
    .Field("b", &TestData::bVal)
    .Field("str", &TestData::strVal)
    .Field("intVal", &TestData::intVal)
    .Field("array", &TestData::array)
    .Field("map", &TestData::map)

    .Method("sum", &TestData::sum)
    .Method("intToStr", &TestData::intToStr)
    .Method("floatToStr", &TestData::floatToStr)
    .Method("boolToStr", &TestData::boolToStr)
    .End();
};

DAVA_TESTCLASS (FormulaExecutorTest)
{
    void SetUp(const String& testName) override
    {
    }

    void TearDown(const String& testName) override
    {
    }

    // FormulaExecutor::Calculate
    DAVA_TEST (CalculateNumbers)
    {
        TEST_VERIFY(Execute("5") == Any(5));
        TEST_VERIFY(Execute("5 + 5") == Any(10));
    }

    // FormulaExecutor::Calculate
    DAVA_TEST (FieldAccess)
    {
        TEST_VERIFY(Execute("map.a") == Any(11));
        TEST_VERIFY(Execute("map.b") == Any(22));
        TEST_VERIFY(Execute("intVal") == Any(42));
        TEST_VERIFY(Execute("map.b + intVal") == Any(22 + 42));
    }

    // FormulaExecutor::Calculate
    DAVA_TEST (IndexAccess)
    {
        TEST_VERIFY(Execute("array[1]") == Any(20));
    }

    // FormulaExecutor::Calculate
    DAVA_TEST (Function)
    {
        TEST_VERIFY(Execute("sum(16, intVal * 2)") == Any(100));
        TEST_VERIFY(Execute("intToStr(55)") == Any(String("*55*")));
        TEST_VERIFY(Execute("floatToStr(55)") == Any(String("55.000")));
        TEST_VERIFY(Execute("floatToStr(55.5)") == Any(String("55.500")));
        TEST_VERIFY(Execute("boolToStr(true)") == Any(String("+")));
    }

    // FormulaExecutor::Calculate
    DAVA_TEST (Dependencies)
    {
        TestData data;

        Vector<void*> dependencies;

        dependencies = GetDependencies("sum(16, intVal * 2)", &data);
        TEST_VERIFY(dependencies == Vector<void*>({ &(data.intVal) }));

        dependencies = GetDependencies("map.b + fl", &data);
        TEST_VERIFY(dependencies == Vector<void*>({ &(data.map), &(data.map["b"]), &(data.flVal) }));

        dependencies = GetDependencies("b && (array[1] == 1)", &data);
        TEST_VERIFY(dependencies == Vector<void*>({ &(data.bVal), &(data.array), &(data.array[1]) }));
    }

    // FormulaExecutor::Calculate
    DAVA_TEST (ParseExpressionWithErrors)
    {
        bool wasException = false;
        try
        {
            Execute("unknownData");
        }
        catch (const FormulaCalculationError& error)
        {
            wasException = true;
        }

        TEST_VERIFY(wasException == true);

        wasException = false;
        try
        {
            Execute("unknownMethod()");
        }
        catch (const FormulaCalculationError& error)
        {
            wasException = true;
        }

        TEST_VERIFY(wasException == true);
    }

    Any Execute(const String str)
    {
        TestData data;
        FormulaReflectionContext context(Reflection::Create(&data), std::shared_ptr<FormulaContext>());
        FormulaExecutor executor(&context);
        FormulaParser parser(str);
        std::shared_ptr<FormulaExpression> exp = parser.ParseExpression();
        Any res = executor.Calculate(exp.get());
        return res;
    }

    Vector<void*> GetDependencies(const String str, TestData* data)
    {
        FormulaReflectionContext context(Reflection::Create(data), std::shared_ptr<FormulaContext>());
        FormulaExecutor executor(&context);
        FormulaParser parser(str);
        std::shared_ptr<FormulaExpression> exp = parser.ParseExpression();

        executor.Calculate(exp.get());
        return executor.GetDependencies();
    }
};
