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

#pragma once

using namespace DAVA;

namespace Testing
{

class TestClass;
class TestClassFactoryBase;

class TestClassFactoryBase
{
public:
    virtual ~TestClassFactoryBase() {}
    virtual TestClass* CreateTestClass() = 0;

protected:
    TestClassFactoryBase() = default;
};

template<typename T>
class TestClassFactoryImpl : public TestClassFactoryBase
{
public:
    TestClass* CreateTestClass() override { return new T; }
};

struct TestInfo
{
    TestInfo(const char* name_, void (*testFunction_)(TestClass*))
        : name(name_)
        , testFunction(testFunction_)
    {}
    String name;
    void (*testFunction)(TestClass*);
};

struct TestClassInfo
{
    TestClassInfo(const char* name_, TestClassFactoryBase* factory_)
        : name(name_)
        , factory(factory_)
    {}
    TestClassInfo(TestClassInfo&& other)
        : name(std::move(other.name))
        , factory(std::move(other.factory))
    {}
    String name;
    std::unique_ptr<TestClassFactoryBase> factory;
};

class TestClass
{
public:
    virtual ~TestClass() {}

    virtual void SetUp(const String& testName) {}
    virtual void TearDown(const String& testName) {}
    virtual void Update(float32 timeElapsed, const String& testName) {}
    virtual bool TestComplete(const String& testName) const { return true; }

    void RegisterTest(const char* name, void (*testFunc)(TestClass*))
    {
        tests.emplace_back(name, testFunc);
    }

    size_t TestCount() const
    {
        return tests.size();
    }

    const String& TestName(size_t index) const
    {
        return tests[index].name;
    }

    void RunTest(size_t index)
    {
        tests[index].testFunction(this);
    }

private:
    Vector<TestInfo> tests;
};

template<typename T>
struct TestClassTypeKeeper
{
    using TestClassType = T;
};

class TestClassCollection
{
public:
    static const size_t npos = size_t(-1);

    static TestClassCollection* Instance()
    {
        static TestClassCollection instance;
        return &instance;
    }

    TestClassCollection() {}
    ~TestClassCollection() {}

    size_t TestClassCount() const
    {
        return testClasses.size();
    }

    const String& TestClassName(size_t index) const
    {
        return testClasses[index].name;
    }

    TestClass* CreateTestClass(size_t index)
    {
        return testClasses[index].factory->CreateTestClass();
    }

    size_t IsTestClassRegistered(const String& name) const
    {
        auto iter = std::find_if(testClasses.begin(), testClasses.end(), [&name](const TestClassInfo& info) -> bool {
            return info.name == name;
        });
        if (iter != testClasses.end())
        {
            return std::distance(testClasses.begin(), iter);
        }
        return npos;
    }

    void RegisterTestClass(const char* name, TestClassFactoryBase* factory)
    {
        testClasses.emplace_back(name, factory);
    }

private:
    Vector<TestClassInfo> testClasses;
};

}   // namespace Testing

#define DAVA_TESTCLASS(classname)                                                                                                   \
    struct classname;                                                                                                               \
    static struct testclass_ ## classname ## _registrar                                                                             \
    {                                                                                                                               \
        testclass_ ## classname ## _registrar()                                                                                     \
        {                                                                                                                           \
            Testing::TestClassCollection::Instance()->RegisterTestClass(#classname, new Testing::TestClassFactoryImpl<classname>);  \
        }                                                                                                                           \
    } testclass_ ## classname ## _registrar_instance;                                                                               \
    struct classname : public Testing::TestClass, public Testing::TestClassTypeKeeper<classname>

#define DAVA_TEST(testname)                                                                                             \
    struct test_ ## testname ## _registrar {                                                                            \
        test_ ## testname ## _registrar(Testing::TestClass* testClass)                                                  \
        {                                                                                                               \
            testClass->RegisterTest(#testname, &test_ ## testname ## _call);                                            \
        }                                                                                                               \
    };                                                                                                                  \
    test_ ## testname ## _registrar test_ ## testname ## _registrar_instance = test_ ## testname ## _registrar(this);   \
    static void test_ ## testname ## _call(TestClass* testClass)                                                        \
    {                                                                                                                   \
        static_cast<TestClassType*>(testClass)->testname();                                                             \
    }                                                                                                                   \
    void testname()
