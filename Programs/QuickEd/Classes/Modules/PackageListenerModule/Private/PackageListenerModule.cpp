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
    using namespace DAVA;
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<PackageListenerProxy>());

    fieldBinder.reset(new FieldBinder(accessor));
    FieldDescriptor fieldDescr;
    fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
    fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
    fieldBinder->BindField(fieldDescr, MakeFunction(this, &PackageListenerModule::OnPackageChanged));
}

void PackageListenerModule::OnPackageChanged(const DAVA::Any& package)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    PackageListenerProxy* proxy = globalContext->GetData<PackageListenerProxy>();

    if (package.CanGet<PackageNode*>())
    {
        proxy->SetPackage(package.Get<PackageNode*>());
    }
    else
    {
        proxy->SetPackage(nullptr);
    }
}
