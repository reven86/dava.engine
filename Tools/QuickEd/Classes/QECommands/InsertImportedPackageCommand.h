#ifndef __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
#define __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__

#include "Document/CommandsBase/QECommand.h"

class PackageNode;
class PackageControlsNode;

class InsertImportedPackageCommand : public QECommand
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
