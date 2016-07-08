#ifndef __QUICKED_CHANGE_STYLE_PROPERTY_COMMAND_H__
#define __QUICKED_CHANGE_STYLE_PROPERTY_COMMAND_H__

#include <QUndoCommand>
#include "FileSystem/VariantType.h"

class PackageNode;
class StyleSheetNode;
class AbstractProperty;

class ChangeStylePropertyCommand : public QUndoCommand
{
public:
    ChangeStylePropertyCommand(PackageNode* _root, StyleSheetNode* _node, AbstractProperty* _property, const DAVA::VariantType& newValue, QUndoCommand* parent = 0);
    virtual ~ChangeStylePropertyCommand();

    void redo() override;
    void undo() override;

private:
    PackageNode* root;
    StyleSheetNode* node;
    AbstractProperty* property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __QUICKED_CHANGE_STYLE_PROPERTY_COMMAND_H__
