#ifndef __DAVAENGINE_TESTCLASS_H__
#define __DAVAENGINE_TESTCLASS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace UnitTests
{
template <typename T>
struct TestClassTypeKeeper
{
    using TestClassType = T;
};

struct CoverageTestInfo
{
    DAVA::Vector<DAVA::String> listTestFile;
    DAVA::Map<DAVA::String, DAVA::String> listTargetFolders;
};

class TestClass
{
    struct TestInfo
    {
        TestInfo(const char* name_, void (*testFunction_)(TestClass*))
            : name(name_)
            , testFunction(testFunction_)
        {
        }
        String name;
        void (*testFunction)(TestClass*);
    };

public:
    TestClass() = default;
    virtual ~TestClass() = default;

    virtual void SetUp(const String& testName);
    virtual void TearDown(const String& testName);
    virtual void Update(float32 timeElapsed, const String& testName);
    virtual bool TestComplete(const String& testName) const;
    virtual void FilesCoveredByTests(CoverageTestInfo&) const;

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

} // namespace UnitTests
} // namespace DAVA

#endif // __DAVAENGINE_TESTCLASS_H__
