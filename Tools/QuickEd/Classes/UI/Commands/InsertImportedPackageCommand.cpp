#include "InsertImportedPackageCommand.h"

#include "UI/Package/PackageModel.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

InsertImportedPackageCommand::InsertImportedPackageCommand(PackageModel *_model, PackageControlsNode *_importedPackageControls, PackageNode *_dest, int _index, QUndoCommand *parent)
    : QUndoCommand(parent)
    , model(_model)
    , importedPackageControls(SafeRetain(_importedPackageControls))
    , dest(SafeRetain(_dest))
    , index(_index)
{
    
}

InsertImportedPackageCommand::~InsertImportedPackageCommand()
{
    SafeRelease(importedPackageControls);
    SafeRelease(dest);
    model = nullptr;
}

void InsertImportedPackageCommand::undo()
{
    model->RemoveImportedPackage(importedPackageControls, dest);
}

void InsertImportedPackageCommand::redo()
{
    model->InsertImportedPackage(importedPackageControls, dest, index);
}
