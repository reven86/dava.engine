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
    bool BaseMe()
    {
        return true;
    }

    DAVA_VIRTUAL_REFLECTION(BaseBase)
    {
        DAVA::ReflectionRegistrator<BaseBase>::Begin()
        .Field("basebase", &BaseBase::basebase)
        .Method("BaseMe", &BaseBase::BaseMe)
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
    std::string str;

protected:
    int baseInt = 123;
    std::string baseStr = "baseStr";
    std::vector<int> intVec;
    SimpleStruct s1;
    const SimpleStruct* simpleNull = nullptr;
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
        .Field("StaticIntFnFn", DAVA::MakeFunction(&TestBaseClass::GetStaticIntFn), DAVA::MakeFunction(&TestBaseClass::SetStaticIntFn))
        .Field("StaticCustomFn", &TestBaseClass::GetStaticCustomFn, nullptr)
        .Field("StaticCustomRefFn", &TestBaseClass::GetStaticCustomRefFn, nullptr)
        .Field("StaticCustomPtrFn", &TestBaseClass::GetStaticCustomPtrFn, nullptr)
        .Field("StaticCustomRefConstFn", &TestBaseClass::GetStaticCustomRefConstFn, nullptr)
        .Field("StaticCustomPtrConstFn", &TestBaseClass::GetStaticCustomPtrConstFn, nullptr)
        .Field("IntFn", &TestBaseClass::GetIntFn, &TestBaseClass::SetIntFn)
        .Field("IntFnFn", DAVA::MakeFunction(&TestBaseClass::GetIntFn), DAVA::MakeFunction(&TestBaseClass::SetIntFn))
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

struct BaseOnlyReflection : public BaseBase
{
    int aaa = 0;

    DAVA_VIRTUAL_REFLECTION(BaseOnlyReflection, BaseBase)
    {
        DAVA::ReflectionRegistrator<BaseOnlyReflection>::Begin().End();
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

DAVA_TESTCLASS (ReflectionTest)
{
    DAVA_TEST (DumpTest)
    {
        TestBaseClass t;
        DAVA::Reflection t_ref = DAVA::Reflection::Create(&t);

        std::ostringstream dumpOutput;
        t_ref.Dump(dumpOutput);

        DAVA::Logger::Info("%s", dumpOutput.str().c_str());

        dumpOutput.clear();
        t_ref.DumpMethods(dumpOutput);

        DAVA::Logger::Info("%s", dumpOutput.str().c_str());

        const TestBaseClass* tptr = &t;
        DAVA::Reflection t_pref = DAVA::Reflection::Create(tptr);

        DAVA::ReflectedObject obj(tptr);

        TEST_VERIFY(t_pref.IsReadonly());
    }

    template <typename T>
    void DoTypeNameTest(const char* permName)
    {
        const DAVA::ReflectedType* rtype0 = DAVA::ReflectedType::Get<T>();
        TEST_VERIFY(nullptr != rtype0);
        TEST_VERIFY(rtype0->GetPermanentName() == permName);
        TEST_VERIFY(rtype0->GetType() == DAVA::Type::Instance<T>());

        const DAVA::ReflectedType* rtype1 = DAVA::ReflectedType::GetByType(DAVA::Type::Instance<T>());
        TEST_VERIFY(rtype1 == rtype0);

        const DAVA::ReflectedType* rtype2 = DAVA::ReflectedType::GetByRttiName(typeid(T).name());
        TEST_VERIFY(rtype2 == rtype0);

        const DAVA::ReflectedType* rtype3 = DAVA::ReflectedType::GetByPermanentName(permName);
        TEST_VERIFY(rtype3 == rtype0);
    }

    DAVA_TEST (TypeNames)
    {
        DoTypeNameTest<BaseBase>("BaseBase");
        DoTypeNameTest<SimpleStruct>("SimpleStruct");
        DoTypeNameTest<TestBaseClass>("TestBaseClass");
    }

    DAVA_TEST (FieldsAndMethods)
    {
        TestBaseClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

        TEST_VERIFY(r.HasFields());
        TEST_VERIFY(r.HasMethods());
        TEST_VERIFY(r.GetField("basebase").ref.IsValid());
        TEST_VERIFY(r.GetMethod("BaseMe").fn.IsValid());

        BaseOnlyReflection b;
        r = DAVA::Reflection::Create(&b);

        TEST_VERIFY(r.HasFields());
        TEST_VERIFY(r.HasMethods());
        TEST_VERIFY(r.GetField("basebase").ref.IsValid());
        TEST_VERIFY(r.GetMethod("BaseMe").fn.IsValid());
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
            TEST_VERIFY(a.Get<T>() == b.Get<T>());

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

        //         rtype = DAVA::ReflectedType::Get<StructPtr>();
        //         TEST_VERIFY(nullptr != rtype->GetCtor());
        //
        //         rtype = DAVA::ReflectedType::Get<int>();
        //         TEST_VERIFY(nullptr != rtype->GetCtor());
        //
        //         rtype = DAVA::ReflectedType::Get<BaseBase>();
        //         TEST_VERIFY(nullptr != rtype->GetCtor());
    }

    template <typename T, typename G, typename S>
    void DoValueSetGetTest(DAVA::Reflection ref, const G& realGetter, const S& realSetter, const T& v1, const T& v2)
    {
        TEST_VERIFY
        (
        ref.GetValueType() == DAVA::Type::Instance<T>() ||
        ref.GetValueType()->Decay() == DAVA::Type::Instance<T>()
        );

        if (ref.GetValueObject().IsValid())
        {
            TEST_VERIFY
            (
            ref.GetValueObject().GetType() == DAVA::Type::Instance<T*>() ||
            ref.GetValueObject().GetType()->Decay() == DAVA::Type::Instance<T*>()
            );
        }

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
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

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
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

        // static get/set
        auto realStaticGetter = DAVA::MakeFunction(&TestBaseClass::GetStaticIntFn);
        auto realStaticSetter = DAVA::MakeFunction(&TestBaseClass::SetStaticIntFn);
        DoValueSetGetTest(r.GetField("StaticIntFn").ref, realStaticGetter, realStaticSetter, 111, 222);
        DoValueSetGetTest(r.GetField("StaticIntFnFn").ref, realStaticGetter, realStaticSetter, 333, 444);

        // class set/get
        auto realClassGetter = DAVA::MakeFunction(&t, &TestBaseClass::GetIntFn);
        auto realClassSetter = DAVA::MakeFunction(&t, &TestBaseClass::SetIntFn);
        DoValueSetGetTest(r.GetField("IntFn").ref, realClassGetter, realClassSetter, 1111, 2222);
        DoValueSetGetTest(r.GetField("IntFnFn").ref, realClassGetter, realClassSetter, 3333, 4444);

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
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

        SimpleStruct s1;
        SimpleStruct s2;

        // class set/get
        auto realClassGetter = [&t]() { return t.simple; };
        auto realClassSetter = [&t](SimpleStruct* s) { t.simple = s; };
        DoValueSetGetTest(r.GetField("simple").ref, realClassGetter, realClassSetter, &s1, &s2);
    }
};

#endif
