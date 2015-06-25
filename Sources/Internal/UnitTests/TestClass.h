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

#ifndef __DAVAENGINE_TESTCLASS_H__
#define __DAVAENGINE_TESTCLASS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace UnitTests
{

template<typename T>
struct TestClassTypeKeeper
{
    using TestClassType = T;
};

class TestClass
{
    struct TestInfo
    {
        TestInfo(const char* name_, void (*testFunction_)(TestClass*))
            : name(name_)
            , testFunction(testFunction_)
        {}
        String name;
        void (*testFunction)(TestClass*);
    };

public:
    TestClass() = default;
    virtual ~TestClass() = default;

    virtual void SetUp(const String& testName) {}
    virtual void TearDown(const String& testName) {}
    virtual void Update(float32 timeElapsed, const String& testName) {}
    virtual bool TestComplete(const String& testName) const { return true; }

    virtual Vector<String> ClassesCoveredByTests() const { return Vector<String>(); }

    const String& TestName(size_t index) const;
    size_t TestCount() const;
    void RunTest(size_t index);

    void RegisterTest(const char* name, void (*testFunc)(TestClass*));

protected:
    String PrettifyTypeName(const String& name) const;
    String RemoveTestPostfix(const String& name) const;

private:
    Vector<TestInfo> tests;
};

//////////////////////////////////////////////////////////////////////////
inline const String& TestClass::TestName(size_t index) const
{
    return tests[index].name;
}

inline size_t TestClass::TestCount() const
{
    return tests.size();
}

inline void TestClass::RunTest(size_t index)
{
    tests[index].testFunction(this);
}

inline void TestClass::RegisterTest(const char* name, void (*testFunc)(TestClass*))
{
    tests.emplace_back(name, testFunc);
}

}   // namespace UnitTests
}   // namespace DAVA

#endif  // __DAVAENGINE_TESTCLASS_H__
