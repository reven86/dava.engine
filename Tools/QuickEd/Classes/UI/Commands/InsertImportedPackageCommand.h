#ifndef __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
#define __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class PackageControlsNode;

class InsertImportedPackageCommand : public QUndoCommand
{
public:
    InsertImportedPackageCommand(PackageNode *_root, PackageControlsNode *_importedPackageControls, int index, QUndoCommand *parent = nullptr);
    virtual ~InsertImportedPackageCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PackageNode *root;
    PackageControlsNode *importedPackageControls;
    int index;
};

#endif // __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
