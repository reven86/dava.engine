#ifndef __QUICKED_INSERT_REMOVE_STYLE_COMMAND_H__
#define __QUICKED_INSERT_REMOVE_STYLE_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class StyleSheetNode;
class StyleSheetsNode;

class InsertRemoveStyleCommand : public QUndoCommand
{
public:
    InsertRemoveStyleCommand(PackageNode* _root, StyleSheetNode* _node, StyleSheetsNode* _dest, int _index, bool insert, QUndoCommand* parent = nullptr);
    virtual ~InsertRemoveStyleCommand();

    void redo() override;
    void undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    StyleSheetsNode* dest;
    int index;
    bool insert;
};

#endif // __QUICKED_INSERT_REMOVE_STYLE_COMMAND_H__
