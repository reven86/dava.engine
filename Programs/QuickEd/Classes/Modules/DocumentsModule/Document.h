#pragma once

#include <TArc/DataProcessing/DataNode.h>
#include <TArc/DataProcessing/DataContext.h>

#include <QString>

namespace DAVA
{
class FilePath;
class CommandStack;
namespace TArc
{
class ContextAccessor;
}
}

struct WidgetContext;
class PackageNode;
class QtModelPackageCommandExecutor;

class Document : public DAVA::TArc::DataNode
{
public:
    explicit Document(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::DataContext::ContextID contextId);
    ~Document();

    const DAVA::FilePath& GetPackageFilePath() const;
    DAVA::CommandStack* GetCommandStack() const;
    PackageNode* GetPackage() const;
    QtModelPackageCommandExecutor* GetCommandExecutor() const;

    WidgetContext* GetContext(void* requester) const;
    void SetContext(void* requester, WidgetContext* widgetContext);

    void SetCanClose(bool val);

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::DataContext::ContextID contextId;

    std::unique_ptr<QtModelPackageCommandExecutor> commandExecutor;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(Document, DAVA::TArc::DataNode)
    {
    }
};
