#include "Test/Private/MockDocumentsModule.h"

#include "Application/QEGlobal.h"

#include <UI/UIControlSystem.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
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
    using namespace DAVA;
    using namespace DAVA::TArc;

    RegisterOperation(QEGlobal::CloseAllDocuments.ID, this, &MockDocumentsModule::CloseAllDocuments);
    RegisterOperation(CreateDummyContextOperation.ID, this, &MockDocumentsModule::CreateDummyContext);

    ContextAccessor* accessor = GetAccessor();
    const EngineContext* engineContext = accessor->GetEngineContext();
    VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
    vcs->UnregisterAllAvailableResourceSizes();
    float32 width = 1.0f;
    float32 height = 1.0f;
    vcs->SetVirtualScreenSize(width, height);
    vcs->RegisterAvailableResourceSize(width, height, "Gfx");
    vcs->RegisterAvailableResourceSize(width, height, "Gfx2");
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

    Vector<std::unique_ptr<TArc::DataNode>> dummy;
    dummy.emplace_back(new MockData());

    ContextManager* manager = GetContextManager();
    DataContext::ContextID id = manager->CreateContext(std::move(dummy));
    manager->ActivateContext(id);
}

IMPL_OPERATION_ID(CreateDummyContextOperation);

} //namespace TestHelpers
