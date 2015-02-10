#include "QtModelPackageCommandExecutor.h"

#include "PackageModelCommands.h"
#include "UIPackageModel.h"

#include "UI/PackageDocument.h"

#include "UIControls/PackageHierarchy/PackageControlsNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/ImportedPackagesNode.h"

using namespace DAVA;

QtModelPackageCommandExecutor::QtModelPackageCommandExecutor(PackageDocument *_document)
    : document(_document)
{
    
}

QtModelPackageCommandExecutor::~QtModelPackageCommandExecutor()
{
    document = nullptr;
}

void QtModelPackageCommandExecutor::InsertControlIntoPackage(ControlNode *control, PackageControlsNode *package)
{
    UIPackageModel *model = document->GetTreeContext()->model;

    QModelIndex dstParent = model->indexByNode(package);
    int32 dstRow = package->GetCount();

    document->UndoStack()->push(new InsertControlNodeCommand(model, control, dstRow, dstParent));
}

void QtModelPackageCommandExecutor::InsertControlIntoParentControl(ControlNode *control, ControlNode *parentControl)
{
    UIPackageModel *model = document->GetTreeContext()->model;
    
    QModelIndex dstParent = model->indexByNode(parentControl);
    int32 dstRow = parentControl->GetCount();
    
    document->UndoStack()->push(new InsertControlNodeCommand(model, control, dstRow, dstParent));
}

void QtModelPackageCommandExecutor::AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package)
{
    UIPackageModel *model = document->GetTreeContext()->model;
    
    QModelIndex dstParent = model->indexByNode(package->GetImportedPackagesNode());
    int32 dstRow = package->GetImportedPackagesNode()->GetCount();
    
    document->UndoStack()->push(new InsertImportedPackageCommand(model, importedPackageControls, dstRow, dstParent));
}
