#include "Modules/LegacySupportModule/Private/Document.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "Modules/ProjectModule/ProjectData.h"

#include "Model/PackageHierarchy/PackageNode.h"

#include <TArc/Core/ContextAccessor.h>

Document::Document(DAVA::TArc::ContextAccessor* accessor_, DAVA::TArc::DataContext::ContextID contextId_)
    : accessor(accessor_)
    , contextId(contextId_)
{
    using namespace DAVA::TArc;
    DataContext* dataContext = accessor->GetContext(contextId);
    const DocumentData* documentData = dataContext->GetData<DocumentData>();
    package = const_cast<PackageNode*>(documentData->GetPackageNode());

    DataContext* globalContext = accessor->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    commandExecutor.reset(new QtModelPackageCommandExecutor(projectData, this));
}

Document::~Document()
{
    for (auto& context : contexts)
    {
        delete context.second;
    }
}

const DAVA::FilePath& Document::GetPackageFilePath() const
{
    using namespace DAVA::TArc;
    const DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    const DocumentData* data = dataContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    const PackageNode* package = data->GetPackageNode();
    return package->GetPath();
}

DAVA::CommandStack* Document::GetCommandStack() const
{
    using namespace DAVA::TArc;
    const DataContext* dataContext = accessor->GetContext(contextId);
    DVASSERT(nullptr != dataContext);
    const DocumentData* data = dataContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);

    return const_cast<DAVA::CommandStack*>(data->GetCommandStack());
}

PackageNode* Document::GetPackage() const
{
    return package;
}

QtModelPackageCommandExecutor* Document::GetCommandExecutor() const
{
    return commandExecutor.get();
}

WidgetContext* Document::GetContext(void* requester) const
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        return iter->second;
    }
    return nullptr;
}

void Document::SetContext(void* requester, WidgetContext* widgetContext)
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        DVASSERT(false, "document already have this context");
        delete iter->second;
        contexts.erase(iter);
    }
    contexts.emplace(requester, widgetContext);
}
