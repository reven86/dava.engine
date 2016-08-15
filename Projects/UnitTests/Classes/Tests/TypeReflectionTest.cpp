#include "Base/Platform.h"

#include <iostream>

#include "Base/Result.h"
#include "Reflection/Registrator.h"
#include "UnitTests/UnitTests.h"
#include "Logger/Logger.h"

using namespace DAVA;

struct SimpleStruct
{
    SimpleStruct()
    {
    }
    SimpleStruct(int a_, int b_)
        : a(a_)
        , b(b_)
    {
    }

    int a = -38;
    int b = 1024;

    DAVA_REFLECTION(SimpleStruct)
    {
        ReflectionRegistrator<SimpleStruct>::Begin()
        .Field("a", &SimpleStruct::a)
        .Field("b", &SimpleStruct::b)
        .End();
    }
};

class BaseBase : public DAVA::ReflectedBase
{
public:
    int basebase = 99;

    DAVA_VIRTUAL_REFLECTION(BaseBase)
    {
        ReflectionRegistrator<BaseBase>::Begin()
        .Field("basebase", &BaseBase::basebase)
        .End();
    }
};

template <typename T>
struct ValueRange
{
    ValueRange(const T& from_, const T& to_)
        : from(from_)
        , to(to_)
    {
    }

    T from;
    T to;
};

template <typename T>
struct ValueValidator
{
    bool IsValid()
    {
        return true;
    }
};

class TestBaseClass : public BaseBase
{
public:
    enum TestEnum
    {
        One,
        Two,
        Three
    };

    TestBaseClass();
    TestBaseClass(int baseInt_, int s_a, int s_b);

    static int staticInt;
    static const int staticIntConst;
    static SimpleStruct staticCustom;

    static int GetStaticIntFn()
    {
        return staticInt;
    }
    static SimpleStruct GetStaticCustomFn()
    {
        return staticCustom;
    }
    static SimpleStruct& GetStaticCustomRefFn()
    {
        return staticCustom;
    }
    static SimpleStruct* GetStaticCustomPtrFn()
    {
        return &staticCustom;
    }
    static const SimpleStruct& GetStaticCustomRefConstFn()
    {
        return staticCustom;
    }
    static const SimpleStruct* GetStaticCustomPtrConstFn()
    {
        return &staticCustom;
    }

    int GetIntFn()
    {
        return baseInt;
    }

    int GetIntFnConst() const
    {
        return baseInt;
    }

    SimpleStruct GetCustomFn()
    {
        return staticCustom;
    }
    SimpleStruct& GetCustomRefFn()
    {
        return staticCustom;
    }
    SimpleStruct* GetCustomPtrFn()
    {
        return &staticCustom;
    }
    const SimpleStruct& GetCustomRefConstFn()
    {
        return staticCustom;
    }

    const SimpleStruct* GetCustomPtrConstFn()
    {
        return &staticCustom;
    }

    TestEnum GetEnum()
    {
        return One;
    }

    int GetEnumAsInt()
    {
        return Two;
    }

    void SetEnum(TestEnum e)
    {
    }

    void SetEnumRef(const TestEnum& e)
    {
    }

protected:
    int baseInt = 123;
    std::string baseStr = "baseStr";
    std::vector<int> intVec;
    SimpleStruct s1;
    SimpleStruct* simple;
    SimpleStruct* simpleNull = nullptr;
    std::vector<std::string> strVec;
    std::vector<SimpleStruct*> simVec;

    DAVA_VIRTUAL_REFLECTION(TestBaseClass, BaseBase)
    {
        ReflectionRegistrator<TestBaseClass>::Begin()
        .Constructor()
        .Constructor<int, int, int>()
        .Destructor()
        .Field("staticInt", &TestBaseClass::staticInt)
        [
        DAVA::Meta<ValueRange<int>>(100, 200),
        DAVA::Meta<ValueValidator<int>>()
        ]
        .Field("staticIntConst", &TestBaseClass::staticIntConst)
        .Field("staticCustom", &TestBaseClass::staticCustom)
        .Field("baseInt", &TestBaseClass::baseInt)
        .Field("baseStr", &TestBaseClass::baseStr)
        .Field("s1", &TestBaseClass::s1)
        .Field("simple", &TestBaseClass::simple)
        .Field("simpleNull", &TestBaseClass::simpleNull)
        .Field("intVec", &TestBaseClass::intVec)
        .Field("strVec", &TestBaseClass::strVec)
        .Field("simVec", &TestBaseClass::simVec)
        .Field("GetStaticIntFn", &TestBaseClass::GetStaticIntFn, nullptr)
        .Field("GetStaticCustomFn", &TestBaseClass::GetStaticCustomFn, nullptr)
        .Field("GetStaticCustomRefFn", &TestBaseClass::GetStaticCustomRefFn, nullptr)
        .Field("GetStaticCustomPtrFn", &TestBaseClass::GetStaticCustomPtrFn, nullptr)
        .Field("GetStaticCustomRefConstFn", &TestBaseClass::GetStaticCustomRefConstFn, nullptr)
        .Field("GetStaticCustomPtrConstFn", &TestBaseClass::GetStaticCustomPtrConstFn, nullptr)
        .Field("GetIntFn", &TestBaseClass::GetIntFn, nullptr)
        .Field("GetIntFnConst", &TestBaseClass::GetIntFnConst, nullptr)
        .Field("GetCustomFn", &TestBaseClass::GetCustomFn, nullptr)
        .Field("GetCustomRefFn", &TestBaseClass::GetCustomRefFn, nullptr)
        .Field("GetCustomPtrFn", &TestBaseClass::GetCustomPtrFn, nullptr)
        .Field("GetCustomRefConstFn", &TestBaseClass::GetCustomRefConstFn, nullptr)
        .Field("GetCustomPtrConstFn", &TestBaseClass::GetCustomPtrConstFn, nullptr)
        .Field("GetEnum", &TestBaseClass::GetEnum, &TestBaseClass::SetEnum)
        .Field("GetGetEnumAsInt", &TestBaseClass::GetEnumAsInt, &TestBaseClass::SetEnumRef)
        .Field("Lambda", Function<int()>([]() { return 1088; }), nullptr)
        .End();
    }
};

int TestBaseClass::staticInt = 222;
const int TestBaseClass::staticIntConst = 888;
SimpleStruct TestBaseClass::staticCustom;

TestBaseClass::TestBaseClass(int baseInt_, int s_a, int s_b)
    : baseInt(baseInt_)
    , s1(s_a, s_b)
{
    simple = &s1;
}

TestBaseClass::TestBaseClass()
{
    static SimpleStruct sss;

    for (int i = 0; i < 5; ++i)
    {
        intVec.push_back(100 - i * 7);

        simVec.push_back(new SimpleStruct(i, 100 - i * 2));
    }

    strVec.push_back("Hello world");
    strVec.push_back("this is dava::reflection");
    strVec.push_back("!!!!!111");

    simple = &sss;
}

DAVA_TESTCLASS (TypeReflection)
{
    DAVA_TEST (DumpTest)
    {
        TestBaseClass t;
        Reflection t_ref = Reflection::Create(&t).ref;

        t_ref.Dump(std::cout);
    }

    DAVA_TEST (CtorDtorTest)
    {
        const ReflectedType* rtype = ReflectedType::Get<TestBaseClass>();
        if (nullptr != rtype)
        {
            // TODO:
            // ...
            /*
            auto ctor = rtype->GetCtor();
            auto dtor = rtype->GetDtor();

            Any a = ctor->Create();
            dtor->Destroy(std::move(a));
            */
        }
    }
};
