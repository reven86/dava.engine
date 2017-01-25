#include "InsertImportedPackageCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

InsertImportedPackageCommand::InsertImportedPackageCommand(PackageNode* aRoot, PackageNode* anImportedPackage, int anIndex)
    : DAVA::Command("InsertImportedPackage")
    , root(SafeRetain(aRoot))
    , importedPackage(SafeRetain(anImportedPackage))
    , index(anIndex)
{
}

InsertImportedPackageCommand::~InsertImportedPackageCommand()
{
    SafeRelease(importedPackage);
    SafeRelease(root);
}

void InsertImportedPackageCommand::Redo()
{
    root->InsertImportedPackage(importedPackage, index);
}

void InsertImportedPackageCommand::Undo()
{
    root->RemoveImportedPackage(importedPackage);
}
