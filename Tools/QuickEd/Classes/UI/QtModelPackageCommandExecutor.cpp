#include "QtModelPackageCommandExecutor.h"

#include "Document.h"
#include "PackageContext.h"

#include "UI/Commands/ChangePropertyValueCommand.h"
#include "UI/Commands/ChangeDefaultValueCommand.h"
#include "UI/Commands/PackageModelCommands.h"
#include "UI/Package/PackageModel.h"

#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"

using namespace DAVA;

QtModelPackageCommandExecutor::QtModelPackageCommandExecutor(Document *_document)
    : document(_document)
{
    
}

QtModelPackageCommandExecutor::~QtModelPackageCommandExecutor()
{
    document = nullptr;
}

void QtModelPackageCommandExecutor::InsertControlIntoPackage(ControlNode *control, PackageControlsNode *package)
{
    PackageModel *model = document->GetPackageContext()->GetModel();

    QModelIndex dstParent = model->indexByNode(package);
    int32 dstRow = package->GetCount();

    document->UndoStack()->push(new InsertControlNodeCommand(model, control, dstRow, dstParent));
}

void QtModelPackageCommandExecutor::InsertControlIntoParentControl(ControlNode *control, ControlNode *parentControl)
{
    PackageModel *model = document->GetPackageContext()->GetModel();
    
    QModelIndex dstParent = model->indexByNode(parentControl);
    int32 dstRow = parentControl->GetCount();
    
    document->UndoStack()->push(new InsertControlNodeCommand(model, control, dstRow, dstParent));
}

void QtModelPackageCommandExecutor::AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package)
{
    PackageModel *model = document->GetPackageContext()->GetModel();
    
    QModelIndex dstParent = model->indexByNode(package->GetImportedPackagesNode());
    int32 dstRow = package->GetImportedPackagesNode()->GetCount();
    
    document->UndoStack()->push(new InsertImportedPackageCommand(model, importedPackageControls, dstRow, dstParent));
}

void QtModelPackageCommandExecutor::ChangeProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &value)
{
    document->UndoStack()->beginMacro("Change Property");
    document->UndoStack()->push(new ChangePropertyValueCommand(property, value));
    const Vector<ControlNode*> &instances = node->GetInstances();
    for (ControlNode *node : instances)
    {
        Vector<String> path = property->GetPath();
        BaseProperty *nodeProperty = node->GetPropertyByPath(path);
        if (nodeProperty)
        {
            document->UndoStack()->push(new ChangeDefaultValueCommand(nodeProperty, value));
        }
        else
        {
            DVASSERT(false);
        }
    }
    document->UndoStack()->endMacro();
}

void QtModelPackageCommandExecutor::ResetProperty(ControlNode *node, BaseProperty *property)
{
    document->UndoStack()->beginMacro("Reset Property");
    document->UndoStack()->push(new ChangePropertyValueCommand(property));
    const Vector<ControlNode*> &instances = node->GetInstances();
    for (ControlNode *node : instances)
    {
        Vector<String> path = property->GetPath();
        BaseProperty *nodeProperty = node->GetPropertyByPath(path);
        if (nodeProperty)
        {
            document->UndoStack()->push(new ChangeDefaultValueCommand(nodeProperty, property->GetDefaultValue()));
        }
        else
        {
            DVASSERT(false);
        }
    }
    document->UndoStack()->endMacro();
}
