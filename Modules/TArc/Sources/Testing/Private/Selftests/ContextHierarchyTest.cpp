#include "Testing/TArcUnitTests.h"

#include "DataProcessing/DataNode.h"

using namespace DAVA::TArc;

class GlobalContextData : public DAVA::TArc::DataNode
{
public:
    DAVA::int32 dummyField;
    
    DAVA_VIRTUAL_REFLECTION(GlobalContextData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<GlobalContextData>::Begin()
            .Field("dummyField", &GlobalContextData::dummyField)
            .End();
    }
};

class SharedData: public DAVA::TArc::DataNode
{
public:
    DAVA::int32 field;
    
    DAVA_VIRTUAL_REFLECTION(SharedData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SharedData>::Begin()
        .Field("field", &SharedData::field)
        .End();
    }
};

DAVA_TARC_TESTCLASS(ContextHierarchyTest)
{
    BEGIN_CLASSES_COVERED_BY_TESTS()
        DECLARE_COVERED_FILES("DataContext")
        DECLARE_COVERED_FILES("TArcCore")
    END_FILES_COVERED_BY_TESTS()
    
    DAVA_TEST(GlobalContexHasDataTest)
    {
        DAVA::TArc::DataContext& ctx = GetGlobalContext();
        TEST_VERIFY(ctx.HasData<GlobalContextData>() == false);
        TEST_VERIFY(ctx.HasData<SharedData>() == false);
        
        ctx.CreateData(std::make_unique<GlobalContextData>());
        TEST_VERIFY(ctx.HasData<GlobalContextData>() == true);
        TEST_VERIFY(GetActiveContext().HasData<GlobalContextData>() == true);
    }
    
    DAVA_TEST(GlobalContextAccessThroughActiveTest)
    {
        try
        {
            GlobalContextData& gd = GetGlobalContext().GetData<GlobalContextData>();
            GlobalContextData& ad = GetActiveContext().GetData<GlobalContextData>();
            TEST_VERIFY(&gd == &ad);
        }
        catch (std::runtime_error& e)
        {
            TEST_VERIFY_WITH_MESSAGE(false, e.what());
        }
    }
    
    DAVA_TEST(GlobalContextDeleteThroughActiveTest)
    {
        DataContext& globalContext = GetGlobalContext();
        DataContext& activeContext = GetActiveContext();
        TEST_VERIFY(globalContext.HasData<GlobalContextData>() == true);
        activeContext.DeleteData<GlobalContextData>();
        TEST_VERIFY(globalContext.HasData<GlobalContextData>() == false);
        TEST_VERIFY(activeContext.HasData<GlobalContextData>() == false);
    }
    
    DAVA_TEST(BothContainsDataTest)
    {
        DataContext& globalContext = GetGlobalContext();
        globalContext.CreateData(std::make_unique<SharedData>());
        
        DataContext& activeContext = GetActiveContext();
        activeContext.CreateData(std::make_unique<SharedData>());
        
        TEST_VERIFY(globalContext.HasData<SharedData>() == true);
        TEST_VERIFY(activeContext.HasData<SharedData>() == true);
        
        SharedData& gd = globalContext.GetData<SharedData>();
        SharedData& ad = activeContext.GetData<SharedData>();
        TEST_VERIFY(&gd != &ad);
        
        activeContext.DeleteData<SharedData>();
        TEST_VERIFY(globalContext.HasData<SharedData>() == true);
        TEST_VERIFY(activeContext.HasData<SharedData>() == true);
        
        globalContext.DeleteData<SharedData>();
        TEST_VERIFY(globalContext.HasData<SharedData>() == false);
        TEST_VERIFY(activeContext.HasData<SharedData>() == false);
    }
    
    DAVA_TEST(ExceptionOnGetDataTest)
    {
        try
        {
            GetGlobalContext().GetData<SharedData>();
        }
        catch(std::runtime_error& e)
        {
            return;
        }
        
        TEST_VERIFY_WITH_MESSAGE(false, "Exception was not throwed");
    }
};







