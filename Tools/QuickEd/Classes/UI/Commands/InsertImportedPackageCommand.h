#ifndef __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
#define __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class PackageControlsNode;

class InsertImportedPackageCommand : public QUndoCommand
{
public:
    InsertImportedPackageCommand(PackageNode *aRoot, PackageNode *anImportedPackage, int anIndex, QUndoCommand *parent = nullptr);
    virtual ~InsertImportedPackageCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PackageNode *root;
    PackageNode *importedPackage;
    int index;
};

#endif // __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
