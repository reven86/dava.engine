#include "Testing/TArcUnitTests.h"
#include "Testing/MockListener.h"

#include "DataProcessing/DataNode.h"

#include "Reflection/ReflectedType.h"
#include "Reflection/Registrator.h"

class DataListenerNode : public DAVA::TArc::DataNode
{
public:
    DAVA::int32 dummyIntField = 0;
    DAVA::float32 dummyFloatField = 0.0f;

    DAVA_VIRTUAL_REFLECTION(DataListenerNode, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<DataListenerNode>::Begin()
        .Field("dummyIntField", &DataListenerNode::dummyIntField)
        .Field("dummyFloatField", &DataListenerNode::dummyFloatField)
        .End();
    }
};

DAVA_TARC_TESTCLASS(DataListenerTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("DataWrapper.cpp")
    DECLARE_COVERED_FILES("DataContext.cpp")
    DECLARE_COVERED_FILES("TArcCore.cpp")
    END_FILES_COVERED_BY_TESTS()

    DAVA_TEST (EmptyDataNodeTest)
    {
        using namespace ::testing;
        DAVA::TArc::DataContext& ctx = GetActiveContext();

        TEST_VERIFY(ctx.HasData<DataListenerNode>() == false);
        ctx.CreateData(std::make_unique<DataListenerNode>());
        TEST_VERIFY(ctx.HasData<DataListenerNode>() == true);

        activeWrapper = CreateWrapper(DAVA::ReflectedType::Get<DataListenerNode>());
        activeWrapper.AddListener(&listener);

        EXPECT_CALL(listener, OnDataChanged(_, DAVA::Set<DAVA::String>{}));
        EXPECT_CALL(secondListener, OnDataChanged(_, DAVA::Set<DAVA::String>{}));
        EXPECT_CALL(bothListener, OnDataChanged(_, DAVA::Set<DAVA::String>{}));

        secondWrapper = CreateWrapper(DAVA::ReflectedType::Get<DataListenerNode>());
        secondWrapper.AddListener(&secondListener);

        bothWrapper = CreateWrapper(DAVA::ReflectedType::Get<DataListenerNode>());
        bothWrapper.AddListener(&bothListener);
    }

    DAVA_TEST (DataNodeValueChangingTest)
    {
        using namespace ::testing;

        DAVA::TArc::DataContext& ctx = GetActiveContext();
        TEST_VERIFY(ctx.HasData<DataListenerNode>() == true);
        ctx.GetData<DataListenerNode>().dummyIntField = 1;

        EXPECT_CALL(listener, OnDataChanged(_, DAVA::Set<DAVA::String>{ "dummyIntField" }));
        EXPECT_CALL(secondListener, OnDataChanged(_, DAVA::Set<DAVA::String>{ "dummyIntField" }));
        EXPECT_CALL(bothListener, OnDataChanged(_, DAVA::Set<DAVA::String>{ "dummyIntField" }));
    }

    DAVA_TEST (CrossDataChangingTest)
    {
        using namespace ::testing;

        TEST_VERIFY(GetActiveContext().HasData<DataListenerNode>() == true);
        EXPECT_CALL(listener, OnDataChanged(_, DAVA::Set<DAVA::String>{ "dummyIntField" }));
        EXPECT_CALL(secondListener, OnDataChanged(_, DAVA::Set<DAVA::String>{ "dummyFloatField" }));
        EXPECT_CALL(bothListener, OnDataChanged(_, DAVA::Set<DAVA::String>{ "dummyIntField", "dummyFloatField" }));

        activeWrapper.CreateEditor<DataListenerNode>()->dummyFloatField = 10.0f;
        secondWrapper.CreateEditor<DataListenerNode>()->dummyIntField = 10;
    }

    DAVA_TEST (DataNodeDeletingTest)
    {
        using namespace ::testing;
        DAVA::TArc::DataContext& ctx = GetActiveContext();
        ctx.DeleteData<DataListenerNode>();

        EXPECT_CALL(listener, OnDataChanged(_, DAVA::Set<DAVA::String>{}));
        EXPECT_CALL(secondListener, OnDataChanged(_, DAVA::Set<DAVA::String>{}));
        EXPECT_CALL(bothListener, OnDataChanged(_, DAVA::Set<DAVA::String>{}));
        TEST_VERIFY(ctx.HasData<DataListenerNode>() == false);
    }

    DAVA::TArc::MockListener listener;
    DAVA::TArc::DataWrapper activeWrapper;

    DAVA::TArc::MockListener secondListener;
    DAVA::TArc::DataWrapper secondWrapper;

    DAVA::TArc::MockListener bothListener;
    DAVA::TArc::DataWrapper bothWrapper;
};