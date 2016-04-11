/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/\

#include <iostream>

#include "Base/Result.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
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
};

class BaseBase : public VirtualReflection
{
    DAVA_DECLARE_TYPE_VIRTUAL_REFLECTION;

public:
    int basebase = 99;
};

class TestBaseClass : public BaseBase
{
    DAVA_DECLARE_TYPE_INITIALIZER;
    DAVA_DECLARE_TYPE_VIRTUAL_REFLECTION;

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

DAVA_TYPE_INITIALIZER(TestBaseClass)
{
    ReflectionRegistrator<BaseBase>::Begin()
    .Field("basebase", &BaseBase::basebase)
    .End();

    ReflectionRegistrator<TestBaseClass>::Begin()
    .Base<BaseBase>()
    .Constructor()
    .Constructor<int, int, int>()
    .Destructor()
    .Field("staticInt", &TestBaseClass::staticInt)
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

    ReflectionRegistrator<SimpleStruct>::Begin()
    .Field("a", &SimpleStruct::a)
    .Field("b", &SimpleStruct::b)
    .End();
}

DAVA_TESTCLASS (TypeReflection)
{
    DAVA_TEST (DumpTest)
    {
        DAVA_TYPE_REGISTER(TestBaseClass);

        TestBaseClass t;
        Reflection t_ref = Reflection::Reflect(&t);

        t_ref.Dump(std::cout);
    }

    DAVA_TEST (CtorDtorTest)
    {
        const ReflectionDB* db = Type::Instance<TestBaseClass>()->GetReflectionDB();
        if (nullptr != db)
        {
            auto ctor = db->GetCtor();
            auto dtor = db->GetDtor();

            Any a = ctor->Create();
            dtor->Destroy(std::move(a));
        }
    }
};
