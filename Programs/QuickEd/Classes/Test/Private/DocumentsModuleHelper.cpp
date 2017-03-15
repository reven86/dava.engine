#include "Test/Private//DocumentsModuleHelper.h"

#include "Application/QEGlobal.h"

#include <Debug/DVAssert.h>

namespace TestHelpers
{
DAVA_VIRTUAL_REFLECTION_IMPL(SampleData)
{
    DAVA::ReflectionRegistrator<SampleData>::Begin()
    .ConstructorByPointer()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentsModuleHelper)
{
    DAVA::ReflectionRegistrator<DocumentsModuleHelper>::Begin()
    .ConstructorByPointer()
    .End();
}

void DocumentsModuleHelper::PostInit()
{
    RegisterOperation(QEGlobal::CloseAllDocuments.ID, this, &DocumentsModuleHelper::CloseAllDocuments);
    RegisterOperation(CreateDummyContextOperation.ID, this, &DocumentsModuleHelper::CreateDummyContext);
}

void DocumentsModuleHelper::OnContextCreated(DAVA::TArc::DataContext* context)
{
}

void DocumentsModuleHelper::OnContextDeleted(DAVA::TArc::DataContext* context)
{
}

void DocumentsModuleHelper::OnRenderSystemInitialized(DAVA::Window* w)
{
}

bool DocumentsModuleHelper::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText)
{
    return true;
}

void DocumentsModuleHelper::SaveOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void DocumentsModuleHelper::RestoreOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void DocumentsModuleHelper::CloseAllDocuments()
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
        SampleData* data = context->GetData<SampleData>();
        if (data != nullptr && data->canClose)
        {
            contextManager->DeleteContext(id);
        }
    }
}

void DocumentsModuleHelper::CreateDummyContext()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    Vector<std::unique_ptr<DataNode>> dummy;
    dummy.emplace_back(new SampleData());

    ContextManager* manager = GetContextManager();
    DataContext::ContextID id = manager->CreateContext(std::move(dummy));
    manager->ActivateContext(id);
}

IMPL_OPERATION_ID(CreateDummyContextOperation);

} //namespace TestHelpers
