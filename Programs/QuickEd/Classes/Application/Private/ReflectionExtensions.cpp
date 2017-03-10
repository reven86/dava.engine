#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/CommonFieldNames.h>

#include <Reflection/ReflectionRegistrator.h>
#include <QString>

namespace ReflectionExtensionsDetail
{
QString GetDocumentName(DAVA::TArc::DataContext* ctx)
{
    DocumentData* data = ctx->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    return data->GetName();
}

QString GetDocumentPath(DAVA::TArc::DataContext* ctx)
{
    DocumentData* data = ctx->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    return data->GetPackageAbsolutePath();
}

bool IsDocumentChanged(DAVA::TArc::DataContext* ctx)
{
    DocumentData* data = ctx->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    return data->CanSave();
}
}

void RegisterDataContextExt()
{
    using namespace ReflectionExtensionsDetail;
    DAVA::ReflectionRegistrator<DAVA::TArc::DataContext>::Begin()
    .Field(DAVA::TArc::ContextNameFieldName, &GetDocumentName, nullptr)
    .Field(DAVA::TArc::ContextToolTipFieldName, &GetDocumentPath, nullptr)
    .Field(DAVA::TArc::IsContextModifiedFieldName, &IsDocumentChanged, nullptr)
    .End();
}

void RegisterReflectionExtensions()
{
    RegisterDataContextExt();
}
