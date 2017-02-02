#include "TArc/Testing/TArcUnitTests.h"

#include "TArc/Core/FieldBinder.h"

#include "Base/BaseTypes.h"
#include "Reflection/ReflectedType.h"

#include <gmock/gmock.h>

using namespace ::testing;

namespace FieldBinderTestDetail
{
class DataNode1 : public DAVA::TArc::DataNode
{
public:
    int v1 = 1;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DataNode1, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<DataNode1>::Begin()
        .Field("v1", &DataNode1::v1)
        .End();
    }
};

class DataNode2 : public DAVA::TArc::DataNode
{
public:
    DAVA::String fv1 = "string";

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DataNode2, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<DataNode2>::Begin()
        .Field("fv1", &DataNode2::fv1)
        .End();
    }
};

class MockObject
{
public:
    MockObject(DAVA::TArc::TestClass* tstClass)
        : testClass(tstClass)
    {
        ON_CALL(*this, ValueChanged(_))
        .WillByDefault(Invoke(this, &MockObject::ValueChangedCallback));
        ON_CALL(*this, ValueChanged2(_))
        .WillByDefault(Invoke(this, &MockObject::ValueChangedCallback2));
    }
    virtual ~MockObject()
    {
    }
    MOCK_METHOD1(ValueChanged, void(const DAVA::Any& v));
    MOCK_METHOD1(ValueChanged2, void(const DAVA::Any& v));

    virtual void ValueChangedCallback(const DAVA::Any& v)
    {
    }
    virtual void ValueChangedCallback2(const DAVA::Any& v)
    {
    }
    virtual void Update() = 0;

    DAVA::TArc::TestClass* testClass;
};

class SingleBindMockObject : public MockObject
{
public:
    SingleBindMockObject(DAVA::TArc::TestClass* testClass)
        : MockObject(testClass)
    {
    }

    ~SingleBindMockObject()
    {
        TEST_VERIFY(phase == 4);
    }

    void ValueChangedCallback(const DAVA::Any& v) override
    {
        if (phase == 0)
        {
            TEST_VERIFY(v.IsEmpty());
            needUpdate = true;
        }
        else if (phase == 1)
        {
            TEST_VERIFY(v.CanCast<int>());
            TEST_VERIFY(v.Cast<int>() == 1);
            needUpdate = true;
        }
        else if (phase == 2)
        {
            TEST_VERIFY(v.CanCast<int>());
            TEST_VERIFY(v.Cast<int>() == 2);
            needUpdate = true;
        }
        else if (phase == 3)
        {
            TEST_VERIFY(v.IsEmpty());
        }

        phase++;
        TEST_VERIFY(phase < 5);
    }

    void Update() override
    {
        if (needUpdate == false)
        {
            return;
        }
        needUpdate = false;

        if (phase == 1)
        {
            testClass->GetAccessor()->GetGlobalContext()->CreateData(std::make_unique<DataNode1>());
        }
        else if (phase == 2)
        {
            testClass->GetAccessor()->GetGlobalContext()->GetData<DataNode1>()->v1 = 2;
        }
        else if (phase == 3)
        {
            testClass->GetAccessor()->GetGlobalContext()->DeleteData<DataNode1>();
        }
        else
        {
            TEST_VERIFY(false);
        }
    }

private:
    bool needUpdate = false;
    int phase = 0;
};

class DualBindMockObject : public MockObject
{
public:
    DualBindMockObject(DAVA::TArc::TestClass* testClass)
        : MockObject(testClass)
    {
    }

    ~DualBindMockObject()
    {
        TEST_VERIFY(nextPhase == 8);
    }

    void ValueChangedCallback(const DAVA::Any& v) override
    {
        if (phase == 0)
        {
            TEST_VERIFY(v.IsEmpty());
            NextPhase();
        }
        else if (phase == 1)
        {
            TEST_VERIFY(v.CanCast<int>());
            TEST_VERIFY(v.Cast<int>() == 1);
            NextPhase();
        }
        else if (phase == 3)
        {
            TEST_VERIFY(v.CanCast<int>());
            TEST_VERIFY(v.Cast<int>() == 2);
            NextPhase();
        }
        else if (phase == 5)
        {
            TEST_VERIFY(v.CanCast<int>());
            TEST_VERIFY(v.Cast<int>() == 3);
            NextPhase();
        }
        else if (phase == 7)
        {
            TEST_VERIFY(v.IsEmpty());
            NextPhase();
        }
    }

    void ValueChangedCallback2(const DAVA::Any& v) override
    {
        if (phase == 0)
        {
            TEST_VERIFY(v.IsEmpty());
            NextPhase();
        }
        else if (phase == 2)
        {
            TEST_VERIFY(v.CanCast<DAVA::String>());
            TEST_VERIFY(v.Cast<DAVA::String>() == "string");
            NextPhase();
        }
        else if (phase == 4)
        {
            TEST_VERIFY(v.CanCast<DAVA::String>());
            TEST_VERIFY(v.Cast<DAVA::String>() == "phase4");
            NextPhase();
        }
        else if (phase == 5)
        {
            TEST_VERIFY(v.CanCast<DAVA::String>());
            TEST_VERIFY(v.Cast<DAVA::String>() == "phase5");
            NextPhase();
        }
        else if (phase == 6)
        {
            TEST_VERIFY(v.IsEmpty());
            NextPhase();
        }
    }

    void Update() override
    {
        using namespace FieldBinderTestDetail;

        if (phase == nextPhase)
        {
            return;
        }
        phase = nextPhase;

        switch (phase)
        {
        case 0:
            break;
        case 1:
            testClass->GetAccessor()->GetGlobalContext()->CreateData(std::make_unique<DataNode1>());
            break;
        case 2:
            testClass->GetAccessor()->GetGlobalContext()->CreateData(std::make_unique<DataNode2>());
            break;
        case 3:
            testClass->GetAccessor()->GetGlobalContext()->GetData<DataNode1>()->v1 = 2;
            break;
        case 4:
            testClass->GetAccessor()->GetGlobalContext()->GetData<DataNode2>()->fv1 = DAVA::String("phase4");
            break;
        case 5:
            testClass->GetAccessor()->GetGlobalContext()->GetData<DataNode1>()->v1 = 3;
            testClass->GetAccessor()->GetGlobalContext()->GetData<DataNode2>()->fv1 = DAVA::String("phase5");
            break;
        case 6:
            testClass->GetAccessor()->GetGlobalContext()->DeleteData<DataNode2>();
            break;
        case 7:
            testClass->GetAccessor()->GetGlobalContext()->DeleteData<DataNode1>();
            break;
        case 8:
            break;
        default:
            TEST_VERIFY(false);
            break;
        }
    }

    void NextPhase()
    {
        nextPhase = phase + 1;
    }

private:
    int nextPhase = 0;
    int phase = 0;
};
}

DAVA_TARC_TESTCLASS(FieldBinderTest)
{
    ~FieldBinderTest()
    {
        delete fieldBinder;
        delete mockObject;
    }

    DAVA_TEST (SingleBindTest)
    {
        mockObject = new FieldBinderTestDetail::SingleBindMockObject(this);
        DAVA::TArc::FieldDescriptor descr;
        descr.fieldName = DAVA::FastName("v1");
        descr.type = DAVA::ReflectedTypeDB::Get<FieldBinderTestDetail::DataNode1>();
        fieldBinder->BindField(descr, DAVA::MakeFunction(mockObject, &FieldBinderTestDetail::MockObject::ValueChanged));
        EXPECT_CALL(*mockObject, ValueChanged(_))
        .Times(4);
    }

    DAVA_TEST (DualBindTest)
    {
        mockObject = new FieldBinderTestDetail::DualBindMockObject(this);
        DAVA::TArc::FieldDescriptor fieldDescr1;
        fieldDescr1.fieldName = DAVA::FastName("v1");
        fieldDescr1.type = DAVA::ReflectedTypeDB::Get<FieldBinderTestDetail::DataNode1>();
        fieldBinder->BindField(fieldDescr1, DAVA::MakeFunction(mockObject, &FieldBinderTestDetail::MockObject::ValueChanged));
        EXPECT_CALL(*mockObject, ValueChanged(_))
        .Times(5);

        DAVA::TArc::FieldDescriptor fieldDescr2;
        fieldDescr2.fieldName = DAVA::FastName("fv1");
        fieldDescr2.type = DAVA::ReflectedTypeDB::Get<FieldBinderTestDetail::DataNode2>();
        fieldBinder->BindField(fieldDescr2, DAVA::MakeFunction(mockObject, &FieldBinderTestDetail::MockObject::ValueChanged2));
        EXPECT_CALL(*mockObject, ValueChanged2(_))
        .Times(5);
    }

    void SetUp(const DAVA::String& testName) override
    {
        DAVA::TArc::TestClass::SetUp(testName);
        if (fieldBinder != nullptr)
        {
            delete fieldBinder;
            fieldBinder = nullptr;
        }

        if (mockObject != nullptr)
        {
            delete mockObject;
            mockObject = nullptr;
        }

        fieldBinder = new DAVA::TArc::FieldBinder(GetAccessor());
    }

    void Update(DAVA::float32 timeElapsed, const DAVA::String& testName) override
    {
        DAVA::TArc::TestClass::Update(timeElapsed, testName);
        if (mockObject != nullptr)
        {
            mockObject->Update();
        }
    }

    DAVA::TArc::FieldBinder* fieldBinder = nullptr;
    FieldBinderTestDetail::MockObject* mockObject = nullptr;

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("FieldBinder.cpp")
    DECLARE_COVERED_FILES("DataWrapper.cpp")
    DECLARE_COVERED_FILES("DataListener.cpp")
    END_FILES_COVERED_BY_TESTS()
};