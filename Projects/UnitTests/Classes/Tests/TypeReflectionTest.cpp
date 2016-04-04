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
=====================================================================================*/

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

class BaseBase : public VirtualReflectionDB
{
    DAVA_DECLARE_VIRTUAL_REFLECTION(BaseBase)

public:
    int basebase = 99;
};

class TestBaseClass : public BaseBase
{
    DAVA_DECLARE_TYPE_INITIALIZER;
    DAVA_DECLARE_VIRTUAL_REFLECTION(TestBaseClass)

public:
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
    .Field("GetCustomFn", &TestBaseClass::GetCustomFn, nullptr)
    .Field("GetCustomRefFn", &TestBaseClass::GetCustomRefFn, nullptr)
    .Field("GetCustomPtrFn", &TestBaseClass::GetCustomPtrFn, nullptr)
    .Field("GetCustomRefConstFn", &TestBaseClass::GetCustomRefConstFn, nullptr)
    .Field("GetCustomPtrConstFn", &TestBaseClass::GetCustomPtrConstFn, nullptr)
    .End();

    ReflectionRegistrator<SimpleStruct>::Begin()
    .Field("a", &SimpleStruct::a)
    .Field("b", &SimpleStruct::b)
    .End();
}

struct type_printer
{
    const DAVA::Type* type;
    void (*print)(char* buf, const DAVA::Any& any);
};

type_printer* get_type_printer(const DAVA::Type* type)
{
    static type_printer pointer_printer = { DAVA::Type::Instance<void*>(), [](char* buf, const DAVA::Any& any) { sprintf(buf, "0x%p", any.Get<void*>()); } };

    static std::vector<type_printer> printers = {
        { DAVA::Type::Instance<int>(), [](char* buf, const DAVA::Any& any) { sprintf(buf, "%d", any.Get<int>()); } },
        { DAVA::Type::Instance<size_t>(), [](char* buf, const DAVA::Any& any) { sprintf(buf, "%lu", any.Get<size_t>()); } },
        { DAVA::Type::Instance<std::string>(), [](char* buf, const DAVA::Any& any) { sprintf(buf, "%s", any.Get<std::string>().c_str()); } }
    };

    type_printer* ret = nullptr;

    if (nullptr != type)
    {
        if (type->IsPointer())
        {
            ret = &pointer_printer;
        }
        else
        {
            for (size_t i = 0; i < printers.size(); ++i)
            {
                if (printers[i].type == type || printers[i].type == type->Decay())
                {
                    ret = &printers[i];
                    break;
                }
            }
        }
    }

    return ret;
}

void print_name_value(const char* name, const char* value, int level)
{
    static char buf[1024];

    int n = 0;
    for (int i = 0; i < level; ++i)
    {
        n += sprintf(buf + n, "  ");
    }

    if (nullptr != name)
    {
        n += sprintf(buf + n, "%s", name);
    }

    for (int i = n; i < 40; ++i)
    {
        n += sprintf(buf + n, " ");
    }

    n += sprintf(buf + n, " : ");

    if (nullptr != value)
    {
        n += sprintf(buf + n, "%s", value);
    }

    //sprintf(buf + n, "\n");
    Logger::Info("%s", buf);
}

const char* anytostr(const DAVA::Any& any)
{
    static char buf[1024];
    buf[0] = 0;

    const DAVA::Type* anytype = any.GetType();
    type_printer* printer = get_type_printer(anytype);

    if (nullptr != printer)
    {
        printer->print(buf, any);
    }
    else
    {
        sprintf(buf, "??? (%.25s)", anytype->GetName());
    }

    return buf;
}

const char* reftostr(const DAVA::Reflection& ref)
{
    const DAVA::Type* type = ref.GetValueType();
    type_printer* printer = get_type_printer(type);

    const char* ret = nullptr;
    if (nullptr != printer && ref.IsValid())
    {
        ret = anytostr(ref.GetValue());
    }
    else
    {
        static char buf[1024];
        sprintf(buf, "??? (%.25s)", type->GetName());
        ret = buf;
    }

    return ret;
}

void dumpref(const std::string& name, const DAVA::Reflection& ref, int level)
{
    print_name_value(name.c_str(), reftostr(ref), level);

    DAVA::ReflectedObject vobject = ref.GetValueObject();
    const DAVA::StructureWrapper* structWrapper = ref.GetStructure();
    if (structWrapper != nullptr)
    {
        DAVA::Ref::FieldsList fields = structWrapper->GetFields(vobject);

        for (size_t i = 0; i < fields.size(); ++i)
        {
            dumpref(anytostr(fields[i].key), fields[i].valueRef, level + 1);
        }
    }
}

DAVA_TESTCLASS (TypeReflection)
{
    DAVA_TEST (DumpTest)
    {
        DAVA_TYPE_REGISTER(TestBaseClass);

        TestBaseClass t;
        Reflection t_ref = Reflection::Reflect(&t);

        dumpref("t", t_ref, 0);
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
