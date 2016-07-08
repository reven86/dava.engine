#ifndef __QUICKED_ADD_REMOVE_STYLE_PROPERTY_COMMAND_H__
#define __QUICKED_ADD_REMOVE_STYLE_PROPERTY_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class StyleSheetNode;
class StyleSheetProperty;

class AddRemoveStylePropertyCommand : public QUndoCommand
{
public:
    AddRemoveStylePropertyCommand(PackageNode* root, StyleSheetNode* node, StyleSheetProperty* property, bool add, QUndoCommand* parent = nullptr);
    virtual ~AddRemoveStylePropertyCommand();

    void redo() override;
    void undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    StyleSheetProperty* property;
    bool add;
};

#endif // __QUICKED_ADD_REMOVE_STYLE_PROPERTY_COMMAND_H__
