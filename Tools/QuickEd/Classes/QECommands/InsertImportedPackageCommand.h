#ifndef __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
#define __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

class PackageNode;
class PackageControlsNode;

class InsertImportedPackageCommand : public CommandWithoutExecute
{
public:
    InsertImportedPackageCommand(PackageNode* aRoot, PackageNode* anImportedPackage, int anIndex);
    virtual ~InsertImportedPackageCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    PackageNode* importedPackage;
    int index;
};

#endif // __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
