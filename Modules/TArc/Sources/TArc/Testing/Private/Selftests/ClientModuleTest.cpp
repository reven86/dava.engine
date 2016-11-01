#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockClientModule.h"

#include "TArc/Core/ControllerModule.h"

using namespace DAVA::TArc;
using namespace ::testing;

struct CMTTag
{
};

class TestControllerModule : public ControllerModule
{
public:
    TestControllerModule()
    {
        DVASSERT(instance == nullptr);
        instance = this;
    }

    static TestControllerModule* instance;

    ContextManager& GetContextMng()
    {
        return GetContextManager();
    }

    ContextAccessor& GetCtxAccessor()
    {
        return GetAccessor();
    }

protected:
    void OnRenderSystemInitialized(DAVA::Window* w) override
    {
    }

    bool CanWindowBeClosedSilently(const WindowKey& key) override
    {
        return true;
    }

    void SaveOnWindowClose(const WindowKey& key) override
    {
    }

    void RestoreOnWindowClose(const WindowKey& key) override
    {
    }

    void OnContextCreated(DataContext& context) override
    {
    }

    void OnContextDeleted(DataContext& context) override
    {
    }

    void OnWindowClosed(const WindowKey& key) override
    {
    }

    void PostInit() override
    {
    }
};

TestControllerModule* TestControllerModule::instance = nullptr;

DAVA_TARC_TESTCLASS(ClientModuleTest)
{
    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(TestControllerModule)
    DECLARE_TESTED_MODULE(MockClientModule<CMTTag>)
    END_TESTED_MODULES()

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("TArcCore.cpp")
    END_FILES_COVERED_BY_TESTS()

    DAVA_TEST (CreateContextTest)
    {
        auto fn = [this](DataContext& ctx)
        {
            undeletedContext = ctx.GetID();
        };

        EXPECT_CALL(*MockClientModule<CMTTag>::instance, OnContextCreated(_))
        .WillOnce(Invoke(fn));

        DataContext::ContextID id = TestControllerModule::instance->GetContextMng().CreateContext();
        TEST_VERIFY(id == undeletedContext);
    }

    DAVA_TEST (ActivateContext)
    {
        DataContext::ContextID newContext = DataContext::Empty;
        auto fn = [this, &newContext](DataContext& ctx)
        {
            newContext = ctx.GetID();
        };

        auto verifyFn = [this, &newContext](DataContext& ctx)
        {
            TEST_VERIFY(newContext == ctx.GetID());
        };

        EXPECT_CALL(*MockClientModule<CMTTag>::instance, OnContextCreated(_))
        .WillOnce(Invoke(fn));
        EXPECT_CALL(*MockClientModule<CMTTag>::instance, OnContextDeleted(_))
        .WillOnce(Invoke(verifyFn));

        ContextAccessor& accessor = TestControllerModule::instance->GetCtxAccessor();
        ContextManager& mng = TestControllerModule::instance->GetContextMng();

        DataContext::ContextID id = mng.CreateContext();
        TEST_VERIFY(id == newContext);

        TEST_VERIFY(accessor.HasActiveContext() == false);

        mng.ActivateContext(undeletedContext);
        TEST_VERIFY(accessor.HasActiveContext() == true);
        TEST_VERIFY(accessor.GetActiveContext().GetID() == undeletedContext);
        TEST_VERIFY(accessor.GetContext(newContext).GetID() == newContext);

        // activate already active context
        mng.ActivateContext(undeletedContext);
        TEST_VERIFY(accessor.HasActiveContext() == true);
        TEST_VERIFY(accessor.GetActiveContext().GetID() == undeletedContext);
        TEST_VERIFY(accessor.GetContext(newContext).GetID() == newContext);

        // deactivate context test
        mng.ActivateContext(DataContext::Empty);
        TEST_VERIFY(accessor.HasActiveContext() == false);

        mng.ActivateContext(undeletedContext);
        TEST_VERIFY(accessor.HasActiveContext() == true);
        TEST_VERIFY(accessor.GetActiveContext().GetID() == undeletedContext);
        TEST_VERIFY(accessor.GetContext(newContext).GetID() == newContext);

        mng.ActivateContext(newContext);
        TEST_VERIFY(accessor.GetActiveContext().GetID() == newContext);

        mng.DeleteContext(newContext);
        TEST_VERIFY(accessor.HasActiveContext() == false);
    }

    DAVA_TEST (DeleteContextTest)
    {
        auto verifyFn = [this](DataContext& ctx)
        {
            TEST_VERIFY(ctx.GetID() == undeletedContext);
        };
        EXPECT_CALL(*MockClientModule<CMTTag>::instance, OnContextDeleted(_))
        .WillOnce(Invoke(verifyFn));

        TestControllerModule::instance->GetContextMng().DeleteContext(undeletedContext);
    }

    DAVA_TEST (DeleteInvalidContextTest)
    {
        bool exeptionCatched = false;
        try
        {
            TestControllerModule::instance->GetContextMng().DeleteContext(1);
        }
        catch (std::runtime_error& /*e*/)
        {
            exeptionCatched = true;
        }

        TEST_VERIFY(exeptionCatched == true);
    }

    DAVA_TEST (ActivateInvalidContextTest)
    {
        bool exeptionCatched = false;
        try
        {
            TestControllerModule::instance->GetContextMng().ActivateContext(1);
        }
        catch (std::runtime_error& /*e*/)
        {
            exeptionCatched = true;
        }

        TEST_VERIFY(exeptionCatched == true);
    }

    DAVA_TEST (GetInvalidContextTest)
    {
        bool exeptionCatched = false;
        try
        {
            TestControllerModule::instance->GetCtxAccessor().GetContext(1);
        }
        catch (std::runtime_error& /*e*/)
        {
            exeptionCatched = true;
        }

        TEST_VERIFY(exeptionCatched == true);
    }

    DAVA_TEST (GetActiveInvalidContextTest)
    {
        bool exeptionCatched = false;
        try
        {
            TestControllerModule::instance->GetCtxAccessor().GetActiveContext();
        }
        catch (std::runtime_error& /*e*/)
        {
            exeptionCatched = true;
        }

        TEST_VERIFY(exeptionCatched == true);
    }

    DataContext::ContextID undeletedContext = DataContext::Empty;
};
