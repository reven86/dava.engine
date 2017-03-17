#include "Test/Private/MockDocumentsModule.h"

#include "Application/QEGlobal.h"

#include <Debug/DVAssert.h>

namespace TestHelpers
{
DAVA_VIRTUAL_REFLECTION_IMPL(MockData)
{
    DAVA::ReflectionRegistrator<MockData>::Begin()
    .ConstructorByPointer()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(MockDocumentsModule)
{
    DAVA::ReflectionRegistrator<MockDocumentsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void MockDocumentsModule::PostInit()
{
    RegisterOperation(QEGlobal::CloseAllDocuments.ID, this, &MockDocumentsModule::CloseAllDocuments);
    RegisterOperation(CreateDummyContextOperation.ID, this, &MockDocumentsModule::CreateDummyContext);
}

void MockDocumentsModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
}

void MockDocumentsModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
}

void MockDocumentsModule::OnRenderSystemInitialized(DAVA::Window* w)
{
}

bool MockDocumentsModule::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText)
{
    return true;
}

void MockDocumentsModule::SaveOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void MockDocumentsModule::RestoreOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void MockDocumentsModule::CloseAllDocuments()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    Vector<DataContext::ContextID> contexts;
    ContextAccessor* accessor = GetAccessor();
    accessor->ForEachContext([&contexts](DataContext& ctx)
                             {
                                 contexts.push_back(ctx.GetID());
                             });

    ContextManager* contextManager = GetContextManager();
    for (DataContext::ContextID id : contexts)
    {
        DataContext* context = accessor->GetContext(id);
        DVASSERT(context != nullptr);
        MockData* data = context->GetData<MockData>();
        if (data != nullptr && data->canClose)
        {
            contextManager->DeleteContext(id);
        }
    }
}

void MockDocumentsModule::CreateDummyContext()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    Vector<std::unique_ptr<DataNode>> dummy;
    dummy.emplace_back(new MockData());

    ContextManager* manager = GetContextManager();
    DataContext::ContextID id = manager->CreateContext(std::move(dummy));
    manager->ActivateContext(id);
}

IMPL_OPERATION_ID(CreateDummyContextOperation);

} //namespace TestHelpers
