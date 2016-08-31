#include "Base/Platform.h"
#ifndef __DAVAENGINE_ANDROID__

#include <iostream>

#include "Base/Result.h"
#include "Reflection/Registrator.h"
#include "UnitTests/UnitTests.h"
#include "Logger/Logger.h"

class StructPtr
{
public:
    StructPtr() = default;
    StructPtr(const StructPtr&) = delete;
    int sss = 555;

    void Release()
    {
        delete this;
    }

protected:
    ~StructPtr() = default;

    DAVA_REFLECTION(StructPtr)
    {
        DAVA::ReflectionRegistrator<StructPtr>::Begin()
        .Field("sss", &StructPtr::sss)
        .End();
    }
};

struct SimpleStruct
{
    SimpleStruct()
    {
    }

    SimpleStruct(int a_)
        : a(a_)
    {
    }

    SimpleStruct(int a_, int b_)
        : a(a_)
        , b(b_)
    {
    }

    SimpleStruct(int a_, int b_, int c_)
        : a(a_)
        , b(b_)
        , c(c_)
    {
    }

    SimpleStruct(int a_, int b_, int c_, int d_)
        : a(a_)
        , b(b_)
        , c(c_)
        , d(d_)
    {
    }

    SimpleStruct(int a_, int b_, int c_, int d_, int e_)
        : a(a_)
        , b(b_)
        , c(c_)
        , d(d_)
        , e(e_)
    {
    }

    int a = -38;
    int b = 1024;
    int c = 1;
    int d = 888;
    int e = 54321;

    bool operator==(const SimpleStruct& s) const
    {
        return (a == s.a && b == s.b);
    }

    DAVA_REFLECTION(SimpleStruct)
    {
        DAVA::ReflectionRegistrator<SimpleStruct>::Begin()
        .Constructor()
        .Constructor<int>()
        .Constructor<int, int>()
        .Constructor<int, int, int>()
        .Constructor<int, int, int, int>()
        .Constructor<int, int, int, int, int>()
        .Destructor()
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
        DAVA::ReflectionRegistrator<BaseBase>::Begin()
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
    ~TestBaseClass();

    static int staticInt;
    static const int staticIntConst;
    static SimpleStruct staticCustom;

    static int GetStaticIntFn()
    {
        return staticInt;
    }

    static void SetStaticIntFn(int v)
    {
        staticInt = v;
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

    void SetIntFn(int v)
    {
        baseInt = v;
    }

    int GetIntFnConst() const
    {
        return baseInt;
    }

    std::string GetBaseStr() const
    {
        return baseStr;
    }

    void SetBaseStr(const std::string& s)
    {
        baseStr = s;
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

    int SumMethod(int a)
    {
        return baseInt + a;
    }

    SimpleStruct* simple;

protected:
    int baseInt = 123;
    std::string baseStr = "baseStr";
    std::vector<int> intVec;
    SimpleStruct s1;
    SimpleStruct* simpleNull = nullptr;
    std::vector<std::string> strVec;
    std::vector<SimpleStruct*> simVec;
    StructPtr* sptr = nullptr;

    DAVA_VIRTUAL_REFLECTION(TestBaseClass, BaseBase)
    {
        DAVA::ReflectionRegistrator<TestBaseClass>::Begin()
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
        .Field("sptr", &TestBaseClass::sptr)
        .Field("StaticIntFn", &TestBaseClass::GetStaticIntFn, &TestBaseClass::SetStaticIntFn)
        .Field("StaticCustomFn", &TestBaseClass::GetStaticCustomFn, nullptr)
        .Field("StaticCustomRefFn", &TestBaseClass::GetStaticCustomRefFn, nullptr)
        .Field("StaticCustomPtrFn", &TestBaseClass::GetStaticCustomPtrFn, nullptr)
        .Field("StaticCustomRefConstFn", &TestBaseClass::GetStaticCustomRefConstFn, nullptr)
        .Field("StaticCustomPtrConstFn", &TestBaseClass::GetStaticCustomPtrConstFn, nullptr)
        .Field("IntFn", &TestBaseClass::GetIntFn, &TestBaseClass::SetIntFn)
        .Field("IntFnConst", &TestBaseClass::GetIntFnConst, nullptr)
        .Field("StrFn", &TestBaseClass::GetBaseStr, &TestBaseClass::SetBaseStr)
        .Field("CustomFn", &TestBaseClass::GetCustomFn, nullptr)
        .Field("CustomRefFn", &TestBaseClass::GetCustomRefFn, nullptr)
        .Field("CustomPtrFn", &TestBaseClass::GetCustomPtrFn, nullptr)
        .Field("CustomRefConstFn", &TestBaseClass::GetCustomRefConstFn, nullptr)
        .Field("CustomPtrConstFn", &TestBaseClass::GetCustomPtrConstFn, nullptr)
        .Field("Enum", &TestBaseClass::GetEnum, &TestBaseClass::SetEnum)
        .Field("GetEnumAsInt", &TestBaseClass::GetEnumAsInt, &TestBaseClass::SetEnumRef)
        .Field("Lambda", DAVA::Function<int()>([]() { return 1088; }), nullptr)
        .Method("SumMethod", &TestBaseClass::SumMethod)
        // .Method("LaMethod", [](const TestBaseClass* t, int a, int b) { return (t->GetIntFnConst() + a - b); })
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
    sptr = new StructPtr();
}

TestBaseClass::~TestBaseClass()
{
    sptr->Release();
}

DAVA_TESTCLASS (TypeReflection)
{
    DAVA_TEST (DumpTest)
    {
        TestBaseClass t;
        DAVA::Reflection t_ref = DAVA::Reflection::Create(&t).ref;

        std::ostringstream dumpOutput;
        t_ref.Dump(dumpOutput);

        DAVA::Logger::Info("%s", dumpOutput.str().c_str());

        dumpOutput.clear();
        t_ref.DumpMethods(dumpOutput);

        DAVA::Logger::Info("%s", dumpOutput.str().c_str());
    }

    DAVA_TEST (FieldsAndMethods)
    {
        TestBaseClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t).ref;

        TEST_VERIFY(r.HasFields());
        TEST_VERIFY(r.HasMethods());
    }

    template <typename T, typename... Args>
    void DoCtorTest(Args... args)
    {
        const DAVA::ReflectedType* rtype = DAVA::ReflectedType::Get<T>();
        const DAVA::CtorWrapper* ctor = rtype->GetCtor(DAVA::AnyFn::Params::FromArgs<Args...>());

        TEST_VERIFY(nullptr != ctor);

        if (nullptr != ctor)
        {
            TEST_VERIFY(ctor->GetInvokeParams().argsType.size() == sizeof...(Args));
            TEST_VERIFY(ctor->GetInvokeParams().retType == DAVA::Type::Instance<void>());

            DAVA::Any a = ctor->Create(DAVA::CtorWrapper::Policy::ByValue, args...);
            DAVA::Any b = T(args...);
            TEST_VERIFY(a == b);

            a = ctor->Create(DAVA::CtorWrapper::Policy::ByPointer, args...);
            TEST_VERIFY(*a.Get<T*>() == b.Get<T>());
        }

        if (sizeof...(Args) != 0)
        {
            // false case, when arguments count doesn't match
            DAVA::Any a = ctor->Create(DAVA::CtorWrapper::Policy::ByValue);
            TEST_VERIFY(a.IsEmpty());
        }
    }

    DAVA_TEST (CtorDtorTest)
    {
        DAVA::Any::RegisterDefaultOPs<SimpleStruct>();

        const DAVA::ReflectedType* rtype = DAVA::ReflectedType::Get<SimpleStruct>();

        TEST_VERIFY(nullptr != rtype);
        if (nullptr != rtype)
        {
            auto ctors = rtype->GetCtors();
            TEST_VERIFY(ctors.size() > 0);

            for (auto& ctor : ctors)
            {
                TEST_VERIFY(ctor != nullptr);
            }

            DoCtorTest<SimpleStruct>();
            DoCtorTest<SimpleStruct>(1);
            DoCtorTest<SimpleStruct>(11, 22);
            DoCtorTest<SimpleStruct>(111, 222, 333);
            DoCtorTest<SimpleStruct>(1111, 2222, 3333, 4444);
            DoCtorTest<SimpleStruct>(11111, 22222, 33333, 44444, 55555);

            const DAVA::DtorWrapper* dtor = rtype->GetDtor();

            TEST_VERIFY(nullptr != dtor);
            if (nullptr != dtor)
            {
                DAVA::Any a = SimpleStruct();

                try
                {
                    dtor->Destroy(std::move(a));
                    TEST_VERIFY(false && "Destroing object created by value shouldn't be able");
                }
                catch (const DAVA::Exception&)
                {
                    TEST_VERIFY(!a.IsEmpty());
                    TEST_VERIFY(a.GetType() == DAVA::Type::Instance<SimpleStruct>());
                }

                a.Set(new SimpleStruct());
                dtor->Destroy(std::move(a));
                TEST_VERIFY(a.IsEmpty());

                DAVA::ReflectedObject obj(new SimpleStruct());
                TEST_VERIFY(obj.IsValid());
                dtor->Destroy(std::move(obj));
                TEST_VERIFY(!obj.IsValid());
            }
        }
    }

    template <typename T, typename G, typename S>
    void DoValueSetGetTest(DAVA::Reflection ref, G & realGetter, S & realSetter, const T& v1, const T& v2)
    {
        TEST_VERIFY(ref.GetValueType() == DAVA::Type::Instance<T>());
        TEST_VERIFY(ref.GetValueObject().GetType() == DAVA::Type::Instance<T*>());
        TEST_VERIFY(ref.GetValueObject().GetType() == DAVA::Type::Instance<T*>());

        DAVA::Any a = ref.GetValue();
        TEST_VERIFY(a.Get<T>() == realGetter());

        if (!ref.IsReadonly())
        {
            realSetter(v1);
            a = ref.GetValue();
            TEST_VERIFY(a.Get<T>() == v1);

            TEST_VERIFY(ref.SetValue(v2));
            TEST_VERIFY(realGetter() == v2);
        }
        else
        {
            TEST_VERIFY(!ref.SetValue(v2));
            TEST_VERIFY(realGetter() != v2);
        }
    }

    DAVA_TEST (ValueSetGet)
    {
        TestBaseClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t).ref;

        // static get/set
        auto realStaticGetter = []() { return TestBaseClass::staticInt; };
        auto realStaticSetter = [](int v) { TestBaseClass::staticInt = v; };
        DoValueSetGetTest(r.GetField("staticInt").ref, realStaticGetter, realStaticSetter, 111, 222);

        // static const get/set
        auto realStaticConstGetter = []() { return TestBaseClass::staticIntConst; };
        auto realStaticConstSetter = [](int v) {};
        DoValueSetGetTest(r.GetField("staticIntConst").ref, realStaticConstGetter, realStaticConstSetter, 111, 222);

        // class set/get
        auto realClassGetter = [&t]() { return t.basebase; };
        auto realClassSetter = [&t](int v) { t.basebase = v; };
        DoValueSetGetTest(r.GetField("basebase").ref, realClassGetter, realClassSetter, 333, 444);
    }

    DAVA_TEST (ValueFnSetGet)
    {
        TestBaseClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t).ref;

        // static get/set
        auto realStaticGetter = DAVA::MakeFunction(&TestBaseClass::GetStaticIntFn);
        auto realStaticSetter = DAVA::MakeFunction(&TestBaseClass::SetStaticIntFn);
        DoValueSetGetTest(r.GetField("StaticIntFn").ref, realStaticGetter, realStaticSetter, 111, 222);

        // class set/get
        auto realClassGetter = DAVA::MakeFunction(&t, &TestBaseClass::GetIntFn);
        auto realClassSetter = DAVA::MakeFunction(&t, &TestBaseClass::SetIntFn);
        DoValueSetGetTest(r.GetField("IntFn").ref, realClassGetter, realClassSetter, 1111, 2222);

        // class const set/get
        auto realClassConstGetter = DAVA::MakeFunction(&t, &TestBaseClass::GetIntFnConst);
        DoValueSetGetTest(r.GetField("IntFnConst").ref, realClassConstGetter, [](int) {}, 1122, 2233);

        // class set/get str
        auto realClassStrGetter = DAVA::MakeFunction(&t, &TestBaseClass::GetBaseStr);
        auto realClassStrSetter = DAVA::MakeFunction(&t, &TestBaseClass::SetBaseStr);
        DoValueSetGetTest(r.GetField("StrFn").ref, realClassStrGetter, realClassStrSetter, std::string("1111"), std::string("2222"));
    }

    DAVA_TEST (ValueSetGetByPointer)
    {
        TestBaseClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t).ref;

        SimpleStruct s1;
        SimpleStruct s2;

        // class set/get
        auto realClassGetter = [&t]() { return t.simple; };
        auto realClassSetter = [&t](SimpleStruct* s) { t.simple = s; };
        DoValueSetGetTest(r.GetField("simple").ref, realClassGetter, realClassSetter, &s1, &s2);
    }
};

#endif
