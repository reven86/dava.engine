#include "QtModelPackageCommandExecutor.h"

#include "Document.h"
#include "PackageContext.h"

#include "UI/PropertiesView/ChangePropertyValueCommand.h"
#include "UI/Package/PackageModelCommands.h"
#include "UI/Package/UIPackageModel.h"

#include "UIControls/PackageHierarchy/PackageControlsNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/ImportedPackagesNode.h"

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
    UIPackageModel *model = document->GetPackageContext()->GetModel();

    QModelIndex dstParent = model->indexByNode(package);
    int32 dstRow = package->GetCount();

    document->UndoStack()->push(new InsertControlNodeCommand(model, control, dstRow, dstParent));
}

void QtModelPackageCommandExecutor::InsertControlIntoParentControl(ControlNode *control, ControlNode *parentControl)
{
    UIPackageModel *model = document->GetPackageContext()->GetModel();
    
    QModelIndex dstParent = model->indexByNode(parentControl);
    int32 dstRow = parentControl->GetCount();
    
    document->UndoStack()->push(new InsertControlNodeCommand(model, control, dstRow, dstParent));
}

void QtModelPackageCommandExecutor::AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package)
{
    UIPackageModel *model = document->GetPackageContext()->GetModel();
    
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
