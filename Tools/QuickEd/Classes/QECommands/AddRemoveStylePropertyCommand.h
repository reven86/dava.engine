#ifndef __QUICKED_ADD_REMOVE_STYLE_PROPERTY_COMMAND_H__
#define __QUICKED_ADD_REMOVE_STYLE_PROPERTY_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

class PackageNode;
class StyleSheetNode;
class StyleSheetProperty;

class AddRemoveStylePropertyCommand : public CommandWithoutExecute
{
public:
    AddRemoveStylePropertyCommand(PackageNode* root, StyleSheetNode* node, StyleSheetProperty* property, bool add);
    virtual ~AddRemoveStylePropertyCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    StyleSheetProperty* property;
    bool add;
};

#endif // __QUICKED_ADD_REMOVE_STYLE_PROPERTY_COMMAND_H__
