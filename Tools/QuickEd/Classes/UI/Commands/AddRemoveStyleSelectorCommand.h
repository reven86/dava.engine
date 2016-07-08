#ifndef __QUICKED_ADD_REMOVE_STYLE_SELECTOR_COMMAND_H__
#define __QUICKED_ADD_REMOVE_STYLE_SELECTOR_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class StyleSheetNode;
class StyleSheetSelectorProperty;

class AddRemoveStyleSelectorCommand : public QUndoCommand
{
public:
    AddRemoveStyleSelectorCommand(PackageNode* root, StyleSheetNode* node, StyleSheetSelectorProperty* property, bool add, QUndoCommand* parent = nullptr);
    virtual ~AddRemoveStyleSelectorCommand();

    void redo() override;
    void undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    StyleSheetSelectorProperty* property;
    bool add;
    int index;
};

#endif // __QUICKED_ADD_REMOVE_STYLE_SELECTOR_COMMAND_H__
