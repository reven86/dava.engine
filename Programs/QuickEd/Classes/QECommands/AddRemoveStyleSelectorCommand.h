#ifndef __QUICKED_ADD_REMOVE_STYLE_SELECTOR_COMMAND_H__
#define __QUICKED_ADD_REMOVE_STYLE_SELECTOR_COMMAND_H__

#include "Command/Command.h"

class PackageNode;
class StyleSheetNode;
class StyleSheetSelectorProperty;

class AddRemoveStyleSelectorCommand : public DAVA::Command
{
public:
    AddRemoveStyleSelectorCommand(PackageNode* root, StyleSheetNode* node, StyleSheetSelectorProperty* property, bool add);
    virtual ~AddRemoveStyleSelectorCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    StyleSheetSelectorProperty* property;
    bool add;
    int index;
};

#endif // __QUICKED_ADD_REMOVE_STYLE_SELECTOR_COMMAND_H__
