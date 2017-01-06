#ifndef __QUICKED_REMOVE_IMPORTED_PACKAGE_COMMAND_H__
#define __QUICKED_REMOVE_IMPORTED_PACKAGE_COMMAND_H__

#include "Command/Command.h"

class PackageNode;
class PackageControlsNode;

class RemoveImportedPackageCommand : public DAVA::Command
{
public:
    RemoveImportedPackageCommand(PackageNode* aRoot, PackageNode* anImportedPackage);
    virtual ~RemoveImportedPackageCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    PackageNode* importedPackage;
    int index;
};

#endif // __QUICKED_REMOVE_IMPORTED_PACKAGE_COMMAND_H__
