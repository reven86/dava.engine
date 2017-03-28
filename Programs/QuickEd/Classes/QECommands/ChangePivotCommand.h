#pragma once

#include "QECommands/Private/QEPackageCommand.h"

#include <FileSystem/VariantType.h>

class ControlNode;
class AbstractProperty;

class ChangePivotCommand : public QEPackageCommand
{
public:
    ChangePivotCommand(PackageNode* package);
    void AddNodePropertyValue(ControlNode* node, AbstractProperty* pivotProperty, const DAVA::VariantType& pivotValue, AbstractProperty* positionProperty, const DAVA::VariantType& positionValue);

    void Redo() override;
    void Undo() override;

    bool MergeWith(const DAVA::Command* command) override;

private:
    struct Item
    {
        Item(ControlNode* node, AbstractProperty* sizeProperty, const DAVA::VariantType& sizeValue, AbstractProperty* pivotProperty, const DAVA::VariantType& pivotValue);
        DAVA::RefPtr<ControlNode> node;
        DAVA::RefPtr<AbstractProperty> pivotProperty;
        DAVA::VariantType pivotNewValue;
        DAVA::VariantType pivotOldValue;

        DAVA::RefPtr<AbstractProperty> positionProperty;
        DAVA::VariantType positionNewValue;
        DAVA::VariantType positionOldValue;
    };
    DAVA::Vector<Item> items;
};
