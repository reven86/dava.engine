#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockClientModule.h"
#include "TArc/Testing/MockControllerModule.h"

#include "Base/Singleton.h"

class CMMockModule : public DAVA::TArc::MockClientModule, public DAVA::Singleton<CMMockModule>
{
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(CMMockModule, DAVA::TArc::MockClientModule, DAVA::Singleton<CMMockModule>)
    {
        DAVA::ReflectionRegistrator<CMMockModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

DAVA_TARC_TESTCLASS(ClientModuleTest)
{
    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(DAVA::TArc::MockControllerModule)
    DECLARE_TESTED_MODULE(CMMockModule)
    END_TESTED_MODULES()

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("TArcCore.cpp")
    END_FILES_COVERED_BY_TESTS()

    DAVA_TEST (CreateContextTest)
    {
        using namespace ::testing;
        using namespace DAVA::TArc;

        auto fn = [this](DataContext* ctx)
        {
            undeletedContext = ctx->GetID();
        };

        EXPECT_CALL(*CMMockModule::Instance(), OnContextCreated(_))
        .WillOnce(Invoke(fn));

        DataContext::ContextID id = GetContextManager()->CreateContext(DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>>());
        TEST_VERIFY(id == undeletedContext);
    }

    DAVA_TEST (ActivateContext)
    {
        using namespace ::testing;
        using namespace DAVA::TArc;

        DataContext::ContextID becomeUnactiveContext = DataContext::Empty;
        DataContext::ContextID becomeActiveContext = DataContext::Empty;
        DataContext::ContextID newContext = DataContext::Empty;
        auto fn = [this, &newContext](DataContext* ctx)
        {
            newContext = ctx->GetID();
        };

        auto willChangedFn = [&](DataContext* ctx, DataContext* newOne)
        {
            if (becomeUnactiveContext == DataContext::Empty)
            {
                TEST_VERIFY(ctx == nullptr);
            }
            else
            {
                TEST_VERIFY(ctx->GetID() == becomeUnactiveContext);
            }

            if (becomeActiveContext == DataContext::Empty)
            {
                TEST_VERIFY(newOne == nullptr);
            }
            else
            {
                TEST_VERIFY(newOne->GetID() == becomeActiveContext);
            }
        };

        auto didChangedFn = [&](DataContext* ctx, DataContext* oldOne)
        {
            if (becomeUnactiveContext == DataContext::Empty)
            {
                TEST_VERIFY(oldOne == nullptr);
            }
            else
            {
                TEST_VERIFY(oldOne->GetID() == becomeUnactiveContext);
            }

            if (becomeActiveContext == DataContext::Empty)
            {
                TEST_VERIFY(ctx == nullptr);
            }
            else
            {
                TEST_VERIFY(ctx->GetID() == becomeActiveContext);
            }
        };

        auto verifyFn = [this, &newContext](DataContext* ctx)
        {
            TEST_VERIFY(newContext == ctx->GetID());
        };

        ON_CALL(*CMMockModule::Instance(), OnContextWillBeChanged(_, _))
        .WillByDefault(Invoke(willChangedFn));
        ON_CALL(*CMMockModule::Instance(), OnContextWasChanged(_, _))
        .WillByDefault(Invoke(didChangedFn));

        EXPECT_CALL(*CMMockModule::Instance(), OnContextCreated(_))
        .WillOnce(Invoke(fn));
        EXPECT_CALL(*CMMockModule::Instance(), OnContextWillBeChanged(_, _))
        .Times(5);
        EXPECT_CALL(*CMMockModule::Instance(), OnContextWasChanged(_, _))
        .Times(5);
        EXPECT_CALL(*CMMockModule::Instance(), OnContextDeleted(_))
        .WillOnce(Invoke(verifyFn));

        ContextAccessor* accessor = GetAccessor();
        ContextManager* mng = GetContextManager();

        auto activateContext = [&](DataContext::ContextID id)
        {
            becomeUnactiveContext = accessor->GetActiveContext() != nullptr ? accessor->GetActiveContext()->GetID() : DataContext::Empty;
            becomeActiveContext = id;
            mng->ActivateContext(id);
        };

        DataContext::ContextID id = mng->CreateContext(DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>>());
        TEST_VERIFY(id == newContext);

        TEST_VERIFY(accessor->GetActiveContext() == nullptr);

        activateContext(undeletedContext);
        TEST_VERIFY(accessor->GetActiveContext() != nullptr);
        TEST_VERIFY(accessor->GetActiveContext()->GetID() == undeletedContext);
        DataContext* dataContext = accessor->GetContext(newContext);
        TEST_VERIFY(dataContext->GetID() == newContext);
        const DataContext* constDataContext = static_cast<const ContextAccessor*>(accessor)->GetContext(newContext);
        TEST_VERIFY(dataContext == constDataContext);

        // activate already active context
        activateContext(undeletedContext);
        TEST_VERIFY(accessor->GetActiveContext() != nullptr);
        TEST_VERIFY(accessor->GetActiveContext()->GetID() == undeletedContext);
        TEST_VERIFY(accessor->GetContext(newContext)->GetID() == newContext);

        // deactivate context test
        activateContext(DataContext::Empty);
        TEST_VERIFY(accessor->GetActiveContext() == nullptr);

        activateContext(undeletedContext);
        TEST_VERIFY(accessor->GetActiveContext() != nullptr);
        TEST_VERIFY(accessor->GetActiveContext()->GetID() == undeletedContext);
        TEST_VERIFY(accessor->GetContext(newContext)->GetID() == newContext);

        activateContext(newContext);
        TEST_VERIFY(accessor->GetActiveContext()->GetID() == newContext);

        // active context deleting produce context switching
        becomeActiveContext = DataContext::Empty;
        becomeUnactiveContext = newContext;
        mng->DeleteContext(newContext);
        TEST_VERIFY(accessor->GetActiveContext() == nullptr);
    }

    DAVA_TEST (DeleteContextTest)
    {
        using namespace ::testing;
        using namespace DAVA::TArc;

        auto verifyFn = [this](DataContext* ctx)
        {
            TEST_VERIFY(ctx->GetID() == undeletedContext);
        };
        EXPECT_CALL(*CMMockModule::Instance(), OnContextDeleted(_))
        .WillOnce(Invoke(verifyFn));

        GetContextManager()->DeleteContext(undeletedContext);
    }

    DAVA_TEST (DeleteInvalidContextTest)
    {
        bool exeptionCatched = false;
        try
        {
            GetContextManager()->DeleteContext(1);
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
            GetContextManager()->ActivateContext(1);
        }
        catch (std::runtime_error& /*e*/)
        {
            exeptionCatched = true;
        }

        TEST_VERIFY(exeptionCatched == true);
    }

    DAVA_TEST (GetConstAccessorTest)
    {
        const DAVA::TArc::ContextAccessor* constAccessor = static_cast<const DAVA::TArc::TestClass*>(this)->GetAccessor();
        DAVA::TArc::ContextAccessor* accessor = GetAccessor();
        TEST_VERIFY(constAccessor != nullptr);
        TEST_VERIFY(constAccessor == accessor);
    }

    DAVA_TEST (GetInvalidContextTest)
    {
        TEST_VERIFY(GetAccessor()->GetContext(1) == nullptr);
    }

    DAVA_TEST (GetActiveInvalidContextTest)
    {
        TEST_VERIFY(GetAccessor()->GetActiveContext() == nullptr);
    }

    DAVA::TArc::DataContext::ContextID undeletedContext = DAVA::TArc::DataContext::Empty;
};
