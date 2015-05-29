#include "InsertImportedPackageCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

InsertImportedPackageCommand::InsertImportedPackageCommand(PackageNode *_root, PackageControlsNode *_importedPackageControls, int _index, QUndoCommand *parent)
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , importedPackageControls(SafeRetain(_importedPackageControls))
    , index(_index)
{
    
}

InsertImportedPackageCommand::~InsertImportedPackageCommand()
{
    SafeRelease(importedPackageControls);
    SafeRelease(root);
}

void InsertImportedPackageCommand::redo()
{
    root->InsertImportedPackage(importedPackageControls, index);
}

void InsertImportedPackageCommand::undo()
{
    root->RemoveImportedPackage(importedPackageControls);
}

