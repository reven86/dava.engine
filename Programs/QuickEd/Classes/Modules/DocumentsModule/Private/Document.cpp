#include "Modules/DocumentsModule/Document.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/WidgetsData.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "Modules/ProjectModule/ProjectData.h"

#include "Model/PackageHierarchy/PackageNode.h"

#include <TArc/Core/ContextAccessor.h>

DAVA_VIRTUAL_REFLECTION_IMPL(Document)
{
}

Document::Document(DAVA::TArc::ContextAccessor* accessor_, DAVA::TArc::DataContext::ContextID contextId_)
    : accessor(accessor_)
    , contextId(contextId_)
{
    using namespace DAVA::TArc;
    DataContext* globalContext = accessor->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    commandExecutor.reset(new QtModelPackageCommandExecutor(projectData, this));
}

Document::~Document() = default;

const DAVA::FilePath& Document::GetPackageFilePath() const
{
    using namespace DAVA::TArc;
    const DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    const DocumentData* data = dataContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    return data->package->GetPath();
}

DAVA::CommandStack* Document::GetCommandStack() const
{
    using namespace DAVA::TArc;
    const DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    const DocumentData* data = dataContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    return data->commandStack.get();
}

PackageNode* Document::GetPackage() const
{
    using namespace DAVA::TArc;
    const DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    const DocumentData* data = dataContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    return data->package.Get();
}

QtModelPackageCommandExecutor* Document::GetCommandExecutor() const
{
    return commandExecutor.get();
}

WidgetContext* Document::GetContext(void* requester) const
{
    using namespace DAVA::TArc;
    DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    WidgetsData* data = dataContext->GetData<WidgetsData>();
    DVASSERT(nullptr != data);
    return data->GetContext(requester);
}

void Document::SetContext(void* requester, WidgetContext* widgetContext)
{
    using namespace DAVA::TArc;
    DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    WidgetsData* data = dataContext->GetData<WidgetsData>();
    DVASSERT(nullptr != data);
    return data->SetContext(requester, widgetContext);
}

void Document::SetCanClose(bool val)
{
    using namespace DAVA::TArc;
    DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    DocumentData* data = dataContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    data->canClose = val;
}
