#ifndef __QUICKED_INSERT_REMOVE_STYLE_COMMAND_H__
#define __QUICKED_INSERT_REMOVE_STYLE_COMMAND_H__

#include "Document/CommandsBase/QECommand.h"

class PackageNode;
class StyleSheetNode;
class StyleSheetsNode;

class InsertRemoveStyleCommand : public QECommand
{
public:
    InsertRemoveStyleCommand(PackageNode* _root, StyleSheetNode* _node, StyleSheetsNode* _dest, int _index, bool insert);
    virtual ~InsertRemoveStyleCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    StyleSheetsNode* dest;
    int index;
    bool insert;
};

#endif // __QUICKED_INSERT_REMOVE_STYLE_COMMAND_H__
