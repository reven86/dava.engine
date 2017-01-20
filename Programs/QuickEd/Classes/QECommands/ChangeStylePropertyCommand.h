#ifndef __QUICKED_CHANGE_STYLE_PROPERTY_COMMAND_H__
#define __QUICKED_CHANGE_STYLE_PROPERTY_COMMAND_H__

#include "Command/Command.h"
#include "Base/Any.h"

class PackageNode;
class StyleSheetNode;
class AbstractProperty;

class ChangeStylePropertyCommand : public DAVA::Command
{
public:
    ChangeStylePropertyCommand(PackageNode* _root, StyleSheetNode* _node, AbstractProperty* _property, const DAVA::Any& newValue);
    virtual ~ChangeStylePropertyCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    AbstractProperty* property;
    DAVA::Any oldValue;
    DAVA::Any newValue;
};

#endif // __QUICKED_CHANGE_STYLE_PROPERTY_COMMAND_H__
