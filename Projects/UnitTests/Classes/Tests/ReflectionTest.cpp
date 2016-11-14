#include <iostream>

#include "Base/Platform.h"
#include "Base/Result.h"
#include "UnitTests/UnitTests.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionQualifier.h"

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
        DAVA::ReflectionQualifier<SimpleStruct>::Begin()
        .ConstructorByValue()
        .ConstructorByValue<int>()
        .ConstructorByValue<int, int>()
        .ConstructorByValue<int, int, int>()
        .ConstructorByValue<int, int, int, int>()
        .ConstructorByValue<int, int, int, int, int>()
        .ConstructorByPointer()
        .DestructorByPointer()
        .Field("a", &SimpleStruct::a)
        .Field("b", &SimpleStruct::b)
        .End();
    }
};

struct A : public virtual DAVA::ReflectedBase
{
    int a = 99;

    bool Me()
    {
        return true;
    }

    DAVA_VIRTUAL_REFLECTION(A)
    {
        DAVA::ReflectionQualifier<A>::Begin()
        .Field("a", &A::a)
        .Method("Me", &A::Me)
        .End();
    }
};

struct B : public virtual DAVA::ReflectedBase
{
    DAVA::String b = "BBB";
    DAVA_VIRTUAL_REFLECTION(B)
    {
        DAVA::ReflectionQualifier<B>::Begin()
        .Field("b", &B::b)
        .End();
    }
};

struct AB : public A, public B
{
    DAVA::String ab = "ABABAB";

    DAVA_VIRTUAL_REFLECTION(AB, A, B)
    {
        DAVA::ReflectionQualifier<AB>::Begin()
        .Field("ab", &AB::ab)
        .End();
    }
};

struct D : public AB
{
    DAVA::String d = "DDD";
    DAVA_VIRTUAL_REFLECTION(D, AB)
    {
        DAVA::ReflectionQualifier<D>::Begin()
        .Field("d", &D::d)
        .End();
    }
};

struct DHolder : DAVA::ReflectedBase
{
    int i;
    D d;

    DAVA_VIRTUAL_REFLECTION(DHolder)
    {
        DAVA::ReflectionQualifier<DHolder>::Begin()
        .Field("i", &DHolder::i)
        .Field("d", &DHolder::d)
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

class ReflectionTestClass : public A
{
public:
    enum TestEnum
    {
        One,
        Two,
        Three
    };

    ReflectionTestClass();
    ReflectionTestClass(int baseInt_, int s_a, int s_b);

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
    std::string baseStr = "TestBaseClass";
    std::vector<int> intVec;
    SimpleStruct s1;
    const SimpleStruct* simpleNull = nullptr;
    std::vector<std::string> strVec;
    std::vector<SimpleStruct*> simVec;
    D* dptr = nullptr;
    A* aptr = nullptr;
    DHolder dholder;

    DAVA_VIRTUAL_REFLECTION(ReflectionTestClass, A)
    {
        DAVA::ReflectionQualifier<ReflectionTestClass>::Begin()
        .ConstructorByPointer()
        .ConstructorByPointer<int, int, int>()
        .DestructorByPointer()
        .Field("staticInt", &ReflectionTestClass::staticInt)[DAVA::Meta<ValueRange<int>>(100, 200), DAVA::Meta<ValueValidator<int>>()]
        .Field("staticIntConst", &ReflectionTestClass::staticIntConst)
        .Field("staticCustom", &ReflectionTestClass::staticCustom)
        .Field("baseInt", &ReflectionTestClass::baseInt)
        .Field("baseStr", &ReflectionTestClass::baseStr)
        .Field("s1", &ReflectionTestClass::s1)
        .Field("simple", &ReflectionTestClass::simple)
        .Field("simpleNull", &ReflectionTestClass::simpleNull)
        .Field("intVec", &ReflectionTestClass::intVec)
        .Field("strVec", &ReflectionTestClass::strVec)
        .Field("simVec", &ReflectionTestClass::simVec)
        .Field("dptr", &ReflectionTestClass::dptr)
        .Field("aptr", &ReflectionTestClass::aptr)
        .Field("dholder", &ReflectionTestClass::dholder)
        .Field("StaticIntFn", &ReflectionTestClass::GetStaticIntFn, &ReflectionTestClass::SetStaticIntFn)
        .Field("StaticIntFnFn", DAVA::MakeFunction(&ReflectionTestClass::GetStaticIntFn), DAVA::MakeFunction(&ReflectionTestClass::SetStaticIntFn))
        .Field("StaticCustomFn", &ReflectionTestClass::GetStaticCustomFn, nullptr)
        .Field("StaticCustomRefFn", &ReflectionTestClass::GetStaticCustomRefFn, nullptr)
        .Field("StaticCustomPtrFn", &ReflectionTestClass::GetStaticCustomPtrFn, nullptr)
        .Field("StaticCustomRefConstFn", &ReflectionTestClass::GetStaticCustomRefConstFn, nullptr)
        .Field("StaticCustomPtrConstFn", &ReflectionTestClass::GetStaticCustomPtrConstFn, nullptr)
        .Field("IntFn", &ReflectionTestClass::GetIntFn, &ReflectionTestClass::SetIntFn)
        .Field("IntFnFn", DAVA::MakeFunction(&ReflectionTestClass::GetIntFn), DAVA::MakeFunction(&ReflectionTestClass::SetIntFn))
        .Field("IntFnConst", &ReflectionTestClass::GetIntFnConst, nullptr)
        .Field("StrFn", &ReflectionTestClass::GetBaseStr, &ReflectionTestClass::SetBaseStr)
        .Field("CustomFn", &ReflectionTestClass::GetCustomFn, nullptr)
        .Field("CustomRefFn", &ReflectionTestClass::GetCustomRefFn, nullptr)
        .Field("CustomPtrFn", &ReflectionTestClass::GetCustomPtrFn, nullptr)
        .Field("CustomRefConstFn", &ReflectionTestClass::GetCustomRefConstFn, nullptr)
        .Field("CustomPtrConstFn", &ReflectionTestClass::GetCustomPtrConstFn, nullptr)
        .Field("Enum", &ReflectionTestClass::GetEnum, &ReflectionTestClass::SetEnum)
        .Field("GetEnumAsInt", &ReflectionTestClass::GetEnumAsInt, &ReflectionTestClass::SetEnumRef)
        .Field("Lambda", DAVA::Function<int()>([]() { return 1088; }), nullptr)
        .Method("SumMethod", &ReflectionTestClass::SumMethod)
        .End();
    }
};

struct BaseOnlyReflection : public A
{
    static BaseOnlyReflection* Create()
    {
        return new BaseOnlyReflection();
    }

    void Release()
    {
        delete this;
    }

    int aaa = 0;

    DAVA_VIRTUAL_REFLECTION(BaseOnlyReflection, A)
    {
        DAVA::ReflectionQualifier<BaseOnlyReflection>::Begin()
        .ConstructorByPointer(&BaseOnlyReflection::Create)
        .DestructorByPointer()
        .End();
    }

private:
    BaseOnlyReflection() = default;
    ~BaseOnlyReflection() = default;
};

int ReflectionTestClass::staticInt = 222;
const int ReflectionTestClass::staticIntConst = 888;
SimpleStruct ReflectionTestClass::staticCustom;

ReflectionTestClass::ReflectionTestClass(int baseInt_, int s_a, int s_b)
    : baseInt(baseInt_)
    , s1(s_a, s_b)
{
    simple = &s1;
}

ReflectionTestClass::ReflectionTestClass()
{
    static SimpleStruct sss;
    static D ddd;

    for (int i = 0; i < 5; ++i)
    {
        intVec.push_back(100 - i * 7);

        simVec.push_back(new SimpleStruct(i, 100 - i * 2));
    }

    strVec.push_back("Hello world");
    strVec.push_back("this is dava::reflection");
    strVec.push_back("!!!!!111");

    simple = &sss;
    dptr = &ddd;
    aptr = &ddd;
}

DAVA_TESTCLASS (ReflectionTest)
{
    DAVA_TEST (DumpTest)
    {
        std::ostringstream dumpOutput;

        ReflectionTestClass t;
        DAVA::Reflection t_ref = DAVA::Reflection::Create(&t);

        t_ref.Dump(dumpOutput);
        DAVA::Logger::Info("%s", dumpOutput.str().c_str());
    }

    template <typename T>
    void DoTypeNameTest(const char* permName)
    {
        const DAVA::ReflectedType* rtype0 = DAVA::ReflectedTypeDB::Get<T>();
        TEST_VERIFY(nullptr != rtype0);
        TEST_VERIFY(rtype0->GetRttiType() == DAVA::RttiType::Instance<T>());

        const DAVA::ReflectedType* rtype1 = DAVA::ReflectedTypeDB::GetByRttiType(DAVA::RttiType::Instance<T>());
        TEST_VERIFY(rtype1 == rtype0);

        const DAVA::ReflectedType* rtype2 = DAVA::ReflectedTypeDB::GetByRttiName(typeid(T).name());
        TEST_VERIFY(rtype2 == rtype0);

        const DAVA::ReflectedType* rtype3 = DAVA::ReflectedTypeDB::GetByPermanentName(permName);
        TEST_VERIFY(rtype3 == rtype0);
    }

    DAVA_TEST (TypeNames)
    {
        DoTypeNameTest<A>("A");
        DoTypeNameTest<SimpleStruct>("SimpleStruct");
        DoTypeNameTest<ReflectionTestClass>("ReflectionTestClass");
    }

    DAVA_TEST (FieldsAndMethods)
    {
        ReflectionTestClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

        TEST_VERIFY(r.HasFields());
        TEST_VERIFY(r.HasMethods());
        TEST_VERIFY(r.GetField("a").IsValid());
        TEST_VERIFY(r.GetMethod("Me").IsValid());

        BaseOnlyReflection* b = BaseOnlyReflection::Create();
        r = DAVA::Reflection::Create(b);
        TEST_VERIFY(r.HasFields());
        TEST_VERIFY(r.HasMethods());
        TEST_VERIFY(r.GetField("a").IsValid());
        TEST_VERIFY(r.GetMethod("Me").IsValid());
        b->Release();

        A* aptr = new D();
        r = DAVA::Reflection::Create(&aptr);
        TEST_VERIFY(r.HasFields());
        TEST_VERIFY(r.GetFields().size() == 4);
        delete aptr;
    }

    template <typename T, typename... Args>
    void DoCtorByValueTest(Args... args)
    {
        const DAVA::ReflectedType* rtype = DAVA::ReflectedTypeDB::Get<T>();

        TEST_VERIFY(true == rtype->HasCtor<Args...>(DAVA::CtorWrapper::Policy::ByValue));

        DAVA::Any a = rtype->Create(DAVA::CtorWrapper::Policy::ByValue, args...);
        DAVA::Any b = T(args...);
        TEST_VERIFY(a.Get<T>() == b.Get<T>());

        a.Clear();

        // false case, when arguments count doesn't match
        try
        {
            a = rtype->Create(DAVA::CtorWrapper::Policy::ByValue, "false case");
            TEST_VERIFY(false && "Invoking ctor with bad arguments shouldn't be able");
        }
        catch (const DAVA::Exception&)
        {
            TEST_VERIFY(a.IsEmpty());
        }
    }

    DAVA_TEST (CtorDtorTest)
    {
        const DAVA::ReflectedType* rtype = DAVA::ReflectedTypeDB::Get<SimpleStruct>();

        TEST_VERIFY(nullptr != rtype);
        if (nullptr != rtype)
        {
            auto ctors = rtype->GetCtors();
            TEST_VERIFY(ctors.size() > 0);

            for (auto& ctor : ctors)
            {
                TEST_VERIFY(ctor != nullptr);
            }

            DoCtorByValueTest<SimpleStruct>();
            DoCtorByValueTest<SimpleStruct>(1);
            DoCtorByValueTest<SimpleStruct>(11, 22);
            DoCtorByValueTest<SimpleStruct>(111, 222, 333);
            DoCtorByValueTest<SimpleStruct>(1111, 2222, 3333, 4444);
            DoCtorByValueTest<SimpleStruct>(11111, 22222, 33333, 44444, 55555);

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
                    TEST_VERIFY(a.GetRttiType() == DAVA::RttiType::Instance<SimpleStruct>());
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

        // custom ctor/dtor
        rtype = DAVA::ReflectedTypeDB::Get<BaseOnlyReflection>();

        DAVA::Any b = rtype->Create(DAVA::CtorWrapper::Policy::ByPointer);
        TEST_VERIFY(!b.IsEmpty());

        rtype->Destroy(std::move(b));
        TEST_VERIFY(b.IsEmpty());

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
        ref.GetValueType() == DAVA::RttiType::Instance<T>() ||
        ref.GetValueType()->Decay() == DAVA::RttiType::Instance<T>()
        );

        if (ref.GetValueObject().IsValid())
        {
            TEST_VERIFY
            (
            ref.GetValueObject().GetType() == DAVA::RttiType::Instance<T*>() ||
            ref.GetValueObject().GetType()->Decay() == DAVA::RttiType::Instance<T*>()
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
        ReflectionTestClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

        // static get/set
        auto realStaticGetter = []() { return ReflectionTestClass::staticInt; };
        auto realStaticSetter = [](int v) { ReflectionTestClass::staticInt = v; };
        DoValueSetGetTest(r.GetField("staticInt"), realStaticGetter, realStaticSetter, 111, 222);

        // static const get/set
        auto realStaticConstGetter = []() { return ReflectionTestClass::staticIntConst; };
        auto realStaticConstSetter = [](int v) {};
        DoValueSetGetTest(r.GetField("staticIntConst"), realStaticConstGetter, realStaticConstSetter, 111, 222);

        // class set/get
        auto realClassGetter = [&t]() { return t.a; };
        auto realClassSetter = [&t](int v) { t.a = v; };
        DoValueSetGetTest(r.GetField("a"), realClassGetter, realClassSetter, 333, 444);
    }

    DAVA_TEST (ValueFnSetGet)
    {
        ReflectionTestClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

        // static get/set
        auto realStaticGetter = DAVA::MakeFunction(&ReflectionTestClass::GetStaticIntFn);
        auto realStaticSetter = DAVA::MakeFunction(&ReflectionTestClass::SetStaticIntFn);
        DoValueSetGetTest(r.GetField("StaticIntFn"), realStaticGetter, realStaticSetter, 111, 222);
        DoValueSetGetTest(r.GetField("StaticIntFnFn"), realStaticGetter, realStaticSetter, 333, 444);

        // class set/get
        auto realClassGetter = DAVA::MakeFunction(&t, &ReflectionTestClass::GetIntFn);
        auto realClassSetter = DAVA::MakeFunction(&t, &ReflectionTestClass::SetIntFn);
        DoValueSetGetTest(r.GetField("IntFn"), realClassGetter, realClassSetter, 1111, 2222);
        DoValueSetGetTest(r.GetField("IntFnFn"), realClassGetter, realClassSetter, 3333, 4444);

        // class const set/get
        auto realClassConstGetter = DAVA::MakeFunction(&t, &ReflectionTestClass::GetIntFnConst);
        DoValueSetGetTest(r.GetField("IntFnConst"), realClassConstGetter, [](int) {}, 1122, 2233);

        // class set/get str
        auto realClassStrGetter = DAVA::MakeFunction(&t, &ReflectionTestClass::GetBaseStr);
        auto realClassStrSetter = DAVA::MakeFunction(&t, &ReflectionTestClass::SetBaseStr);
        DoValueSetGetTest(r.GetField("StrFn"), realClassStrGetter, realClassStrSetter, std::string("1111"), std::string("2222"));
    }

    DAVA_TEST (ValueSetGetByPointer)
    {
        ReflectionTestClass t;
        DAVA::Reflection r = DAVA::Reflection::Create(&t);

        SimpleStruct s1;
        SimpleStruct s2;

        // class set/get
        auto realClassGetter = [&t]() { return t.simple; };
        auto realClassSetter = [&t](SimpleStruct* s) { t.simple = s; };
        DoValueSetGetTest(r.GetField("simple"), realClassGetter, realClassSetter, &s1, &s2);

        const ReflectionTestClass* tptr = &t;
        DAVA::Reflection t_pref = DAVA::Reflection::Create(tptr);
    }
};
