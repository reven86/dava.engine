#include "Modules/PackageListenerModule/PackageListenerModule.h"
#include "Modules/PackageListenerModule/PackageListenerProxy.h"
#include "Modules/DocumentsModule/DocumentData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PackageListenerModule)
{
    DAVA::ReflectionRegistrator<PackageListenerModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void PackageListenerModule::PostInit()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<PackageListenerProxy>());
}

void PackageListenerModule::OnContextWillBeChanged(DAVA::TArc::DataContext* /*current*/, DAVA::TArc::DataContext* /*newOne*/)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    PackageListenerProxy* proxy = globalContext->GetData<PackageListenerProxy>();
    proxy->SetPackage(nullptr);
}

void PackageListenerModule::OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* /*oldOne*/)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    PackageListenerProxy* proxy = globalContext->GetData<PackageListenerProxy>();
    if (current != nullptr)
    {
        DocumentData* documentData = current->GetData<DocumentData>();
        DVASSERT(nullptr != documentData);
        proxy->SetPackage(documentData->GetPackageNode());
    }
    else
    {
        proxy->SetPackage(nullptr);
    }
}
