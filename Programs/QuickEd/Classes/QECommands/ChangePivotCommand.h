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

    ~ChangePivotCommand() override = default;

    void Redo() override;
    void Undo() override;

    bool MergeWith(const DAVA::Command* command) override;

private:
    struct Item
    {
        Item(ControlNode* node, AbstractProperty* sizeProperty, const DAVA::VariantType& sizeValue, AbstractProperty* pivotProperty, const DAVA::VariantType& pivotValue);
        ControlNode* node = nullptr;
        AbstractProperty* pivotProperty = nullptr;
        DAVA::VariantType pivotNewValue;
        DAVA::VariantType pivotOldValue;

        AbstractProperty* positionProperty = nullptr;
        DAVA::VariantType positionNewValue;
        DAVA::VariantType positionOldValue;
    };
    DAVA::Vector<Item> items;
};
