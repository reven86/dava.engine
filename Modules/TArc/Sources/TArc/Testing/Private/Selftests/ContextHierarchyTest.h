#include "TArc/Testing/TArcUnitTests.h"

#include "TArc/DataProcessing/DataNode.h"

using namespace DAVA::TArc;

class GlobalContextData : public DAVA::TArc::DataNode
{
public:
    DAVA::int32 dummyField;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(GlobalContextData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<GlobalContextData>::Begin()
        .Field("dummyField", &GlobalContextData::dummyField)
        .End();
    }
};

class SharedData : public DAVA::TArc::DataNode
{
public:
    DAVA::int32 field;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SharedData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SharedData>::Begin()
        .Field("field", &SharedData::field)
        .End();
    }
};

DAVA_TARC_TESTCLASS(ContextHierarchyTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("DataContext.cpp")
    DECLARE_COVERED_FILES("TArcCore.cpp")
    END_FILES_COVERED_BY_TESTS();

    DAVA_TEST (GlobalContexHasDataTest)
    {
        DAVA::TArc::DataContext* ctx = GetGlobalContext();
        TEST_VERIFY(ctx->GetData<GlobalContextData>() == nullptr);
        TEST_VERIFY(ctx->GetData<SharedData>() == nullptr);

        ctx->CreateData(std::make_unique<GlobalContextData>());
        TEST_VERIFY(ctx->GetData<GlobalContextData>() != nullptr);
        TEST_VERIFY(GetActiveContext()->GetData<GlobalContextData>() != nullptr);
    }

    DAVA_TEST (GlobalContextAccessThroughActiveTest)
    {
        try
        {
            GlobalContextData* gd = GetGlobalContext()->GetData<GlobalContextData>();
            GlobalContextData* ad = GetActiveContext()->GetData<GlobalContextData>();
            TEST_VERIFY(gd == ad);
        }
        catch (std::runtime_error& e)
        {
            TEST_VERIFY_WITH_MESSAGE(false, e.what());
        }
    }

    DAVA_TEST (GlobalContextDeleteThroughActiveTest)
    {
        DataContext* globalContext = GetGlobalContext();
        DataContext* activeContext = GetActiveContext();
        TEST_VERIFY(globalContext->GetData<GlobalContextData>() != nullptr);
        activeContext->DeleteData<GlobalContextData>();
        TEST_VERIFY(globalContext->GetData<GlobalContextData>() == nullptr);
        TEST_VERIFY(activeContext->GetData<GlobalContextData>() == nullptr);
    }

    DAVA_TEST (BothContainsDataTest)
    {
        DataContext* globalContext = GetGlobalContext();
        globalContext->CreateData(std::make_unique<SharedData>());

        DataContext* activeContext = GetActiveContext();
        activeContext->CreateData(std::make_unique<SharedData>());

        TEST_VERIFY(globalContext->GetData<SharedData>() != nullptr);
        TEST_VERIFY(activeContext->GetData<SharedData>() != nullptr);

        SharedData* gd = globalContext->GetData<SharedData>();
        SharedData* ad = activeContext->GetData<SharedData>();
        TEST_VERIFY(gd != ad);

        activeContext->DeleteData<SharedData>();
        TEST_VERIFY(globalContext->GetData<SharedData>() != nullptr);
        TEST_VERIFY(activeContext->GetData<SharedData>() != nullptr);

        globalContext->DeleteData<SharedData>();
        TEST_VERIFY(globalContext->GetData<SharedData>() == nullptr);
        TEST_VERIFY(activeContext->GetData<SharedData>() == nullptr);
    }

    DAVA_TEST (ConstDataContextTest)
    {
        using namespace DAVA::TArc;
        DataContext* ctx = GetGlobalContext();
        const DataContext* constCtx = static_cast<const DAVA::TArc::TestClass*>(this)->GetGlobalContext();
        TEST_VERIFY(ctx != nullptr);
        TEST_VERIFY(constCtx == ctx);

        DataContext* activeCtx = GetActiveContext();
        const DataContext* constActiveCtx = static_cast<const DAVA::TArc::TestClass*>(this)->GetActiveContext();
        TEST_VERIFY(activeCtx != nullptr);
        TEST_VERIFY(constActiveCtx == activeCtx);
    }
}
;
